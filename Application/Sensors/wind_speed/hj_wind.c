
#include "hj_wind.h"

#include <math.h>
#include <string.h>

#include "app_rs485.h"
#include "app_sensor.h"
#include "cmsis_os2.h"
#include "dev_io.h"
#include "driver_485.h"
#include "driver_uart.h"
#include "util_memory.h"
#include "config_sensor.h"

typedef struct hjwind_cfg_s
{
  driver_t *rs485_io;
} hjwind_cfg_t;


void set_hjwind(void *handle, wind_set_option_t option, void *value);
float read_hjwind(void *driver, uint8_t type, uint8_t *err);
void set_hjwind(void *handle, wind_set_option_t option, void *value);



/* AWS AVR 에서 가져옴
  화진티엔아이 AWS 풍향 풍속계  PROTOCOL 정의
        .Data Table.
        Start Code     	0  	: 0x02 			-> STX
        Unit ID		   	1	: 0x01 - 0x0f 	-> 장비 ID
        Command			2	: 0xXX          -> 01:파라메타 설정, 02:Data Read, 03: Write
  & Read Data Size       4	: 0x02			-> 데이터의 사이즈 Data            5	: n
  -> 전송되는 데이터 ASCII 형식 Check Sum		6	: 1      		-> ID - Data
  n 까지의 합 End Code 		7	: 0x03			-> ETX
*/

// 02 01 02 01 01 05 03   풍속
// 02 02 02 01 01 06 03   풍향
uint16_t make_hjwind(uint8_t *sSend, uint8_t id)
{
  uint8_t cnt = 0;
  uint8_t len;

  sSend[cnt++] = 0x02;
  sSend[cnt++] = id;    // 01:Wind Speed ID, 02:Wind Direction
  sSend[cnt++] = 0x02;  // Data Read Command
  sSend[cnt++] = 0x01;  // Data Length
  sSend[cnt++] = 0x01;  // Dummy Data
  len = cnt - 1;
  sSend[cnt++] = make_sum(&sSend[1], len);
  sSend[cnt++] = 0x03;

  return (cnt);
}

bool is_hjwin(uint8_t *frame, uint16_t len)
{
  uint8_t sum = 0;

  if (frame[0] == 0x02)
  {
    sum = make_sum(&frame[1], frame[3] + 3);
    if (sum == frame[len - 2])
    {
      return true;
    }
  }

  return false;
}

#define WIND_DATA_MAX 9990
//이코드는 구형 AWS코드와 동일
//풍속센서의 값은 풍속값자체가 아닌 펄스값임
float calculate_wind_speed(uint16_t wind_pulse)
{
  int32_t tmp = 0;
  float wind_speed = 0;
  int32_t span;
  uint32_t errTmp;

  float gain;
  uint16_t offset;
  uint16_t fullset;

  offset = get_config_sensor()->hjwind[0].offset;
  fullset = get_config_sensor()->hjwind[0].full;

  if (wind_pulse < WIND_DATA_MAX)
  {
    span = fullset - offset;

    errTmp = (uint32_t)((float)span * 0.05);  // offset보다 5% 크고 Full보다 5% 작을 것

    if (wind_pulse < (fullset + errTmp))
    {
      if (span > 0)
      {
        gain = 70.0 / (float)span;
        wind_speed = ((float)(wind_pulse - offset) * gain);
      }
    }
    else
    {
      wind_speed = 0;
    }
  }

  return wind_speed;
}



float read_hjwind(void *driver, uint8_t channel, uint8_t *err)
{
  uint8_t send[10];
  uint8_t recv[10];
  uint16_t len;
  uint16_t windData = 0;
  float retVal = NAN;
  

  hjwind_cfg_t *cfg = ((driver_t *)driver)->cfg;

  len = make_hjwind(send, channel);

  driver_rs485_flush_rx(cfg->rs485_io);
  driver_rs485_send(cfg->rs485_io, send, len);
   
  // 독라이트가 응답을항상 일정한 시간안에 보내는것이 아님
  // 2ms 안에 응답오는 경우도 있고 50ms 지나고 응답 오는 경우도 있음
  // 따라서 독라이트 테스트시에는 첫번째 바이트 대기 시간을 50ms 해야 수신 처리됨


  len = driver_rs485_recv_opt(cfg->rs485_io, recv, sizeof(recv), 50,5); 

  if(len == 0)
  {
    *err = DRV_ERR_TIMEOUT;
    return NAN;
  }

  if (len && is_hjwin(recv, len))
  {
    if (channel == HJ_WIND_CHANNEL_DIRECTION)
    {
      if (((recv[1] & 0x7f) == 0x02))
      {
        *err = DRV_ERR_NONE;
        memcpy(&windData, &recv[4], 2);
      }
    }
    else if (channel == HJ_WIND_CHANNEL_SPEED)
    {
      if (((recv[1] & 0x7f) == 0x01))
      {
        *err = DRV_ERR_NONE;
        memcpy(&windData, &recv[4], 2);
        return calculate_wind_speed(windData); 
      }
    }
  }
  else
  {
    *err = DRV_ERR_RECV_DATA;
    return NAN;
  }

  return (float)((float)windData / 10.0);//풍향은 10배 된 값이 수신됨
  //풍향1234가 수신 -> 123.4도임 따라서 드라이버의 값의 단위는 도임, 따라서 10으로 나눈값을 리턴턴

}


void get_hjwind(driver_t *drv, int cmd, void *val)
{

}

void set_hjwind(void *handle, wind_set_option_t option, void *value)
{


}

wind_api_t hjwind_api = {.read = read_hjwind, .set = set_hjwind};
hjwind_cfg_t g_hjwind_cfg;
driver_t g_hjwind_driver;

/**
 * @details
 * 고정된 속도로 사용하는 센서들은 포트설정만 매개변수로 받아서 처리
 */
driver_t *hjwind_open(uint8_t num, void *opt)
{
  uart_config_t uart_config;
  hjwindspeed_config_t *hjwind_config = opt;

  (void)num;

  if (g_hjwind_driver.opened == true)
  {
    return &g_hjwind_driver;
  }

  uart_config.baud = 9600;
  uart_config.parityIdx = 0;
  uart_config.dataLen = 8;
  uart_config.stop_bit = 1;

  g_hjwind_cfg.rs485_io = driver_rs485_open((int)hjwind_config->rs485_port, &uart_config);

  g_hjwind_driver.name = "hj_wind";
  g_hjwind_driver.cfg = &g_hjwind_cfg;
  g_hjwind_driver.api = &hjwind_api;

  g_hjwind_driver.opened = true;

  return &g_hjwind_driver;
}