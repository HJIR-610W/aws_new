
#include "hj_wind.h"

#include <math.h>
#include <string.h>

#include "app_rs485.h"
#include "app_sensor.h"
#include "cmsis_os2.h"
#include "dev_io.h"
#include "driver_485.h"
#include "driver_uart.h"
#include "utile.h"
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






float read_hjwind(void *driver, uint8_t channel, uint8_t *err)
{
  uint8_t send[10];
  uint8_t recv[10];
  uint16_t len;
  uint16_t windData = 0;
  float retVal = NAN;

  hjwind_cfg_t *cfg = ((driver_t *)driver)->cfg;

  len = make_hjwind(send, channel);

  driver_rs485_send(cfg->rs485_io, send, len);

  // 10배된 값이 수신됨됨
  len = driver_rs485_recv_opt(cfg->rs485_io, recv, sizeof(recv),
                              10,5); //응답이 10ms 안에 와야하며, 5ms 이상 데이터 미 수신시 종료

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
      }
    }
  }
  else
  {
    *err = DRV_ERR_RECV_DATA;
    return NAN;
  }

  return (float)((float)windData / 10.0);

}


void get_hjwind(driver_t *drv, int cmd, void *val)
{

}

void set_hjwind(void *handle, wind_set_option_t option, void *value)
{


}

wind_api_t hjwind_api = {.read = read_hjwind, .set = set_hjwind};
hjwind_cfg_t g_hjwind_cfg;
driver_t g_hjwind;

driver_t *hjwind_open(uint8_t num, void *opt)
{
  uart_config_t uart_config;
  hjwindspeed_config_t *hjwind_config = opt;

  if (g_hjwind.opened == true)
  {
    return &g_hjwind;
  }

  uart_config.baud = 9600;
  uart_config.parityIdx = 0;
  uart_config.dataLen = 8;
  uart_config.stop_bit = 1;

  g_hjwind_cfg.rs485_io = driver_rs485_open((int)hjwind_config->rs485_port, &uart_config);

  g_hjwind.cfg = &g_hjwind_cfg;
  g_hjwind.api = &hjwind_api;
  g_hjwind.opened = true;

  return &g_hjwind;
}