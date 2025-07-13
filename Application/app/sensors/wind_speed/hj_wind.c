
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



/* AWS AVR ?먯꽌 媛?몄샂
  ?붿쭊?곗뿏?꾩씠 AWS ?랁뼢 ?띿냽怨? PROTOCOL ?뺤쓽
        .Data Table.
        Start Code     	0  	: 0x02 			-> STX
        Unit ID		   	1	: 0x01 - 0x0f 	-> ?λ퉬 ID
        Command			2	: 0xXX          -> 01:?뚮씪硫뷀? ?ㅼ젙, 02:Data Read, 03: Write
  & Read Data Size       4	: 0x02			-> ?곗씠?곗쓽 ?ъ씠利?Data            5	: n
  -> ?꾩넚?섎뒗 ?곗씠??ASCII ?뺤떇 Check Sum		6	: 1      		-> ID - Data
  n 源뚯?????End Code 		7	: 0x03			-> ETX
*/

// 02 01 02 01 01 05 03   ?띿냽
// 02 02 02 01 01 06 03   ?랁뼢
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
//?댁퐫?쒕뒗 援ы삎 AWS肄붾뱶? ?숈씪
//?띿냽?쇱꽌??媛믪? ?띿냽媛믪옄泥닿? ?꾨땶 ?꾩뒪媛믪엫
float calculate_wind_speed(uint16_t wind_pulse)
{

  float wind_speed = 0;
  int32_t span;
  uint32_t errTmp;

  float gain;
  uint16_t offset;
  uint16_t fullset;

  offset = get_config_sensor()->hjwind_speed.offset;
  fullset = get_config_sensor()->hjwind_speed.full;

  if (wind_pulse < WIND_DATA_MAX)
  {
    span = fullset - offset;

    errTmp = (uint32_t)((float)span * 0.05);  // offset蹂대떎 5% ?ш퀬 Full蹂대떎 5% ?묒쓣 寃?

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

  

  hjwind_cfg_t *cfg = ((driver_t *)driver)->cfg;

  len = make_hjwind(send, channel);

  driver_rs485_flush_rx(cfg->rs485_io);
  driver_rs485_send(cfg->rs485_io, send, len);
   
  // ?낅씪?댄듃媛 ?묐떟?꾪빆???쇱젙???쒓컙?덉뿉 蹂대궡?붽쾬???꾨떂
  // 2ms ?덉뿉 ?묐떟?ㅻ뒗 寃쎌슦???덇퀬 50ms 吏?섍퀬 ?묐떟 ?ㅻ뒗 寃쎌슦???덉쓬
  // ?곕씪???낅씪?댄듃 ?뚯뒪?몄떆?먮뒗 泥ル쾲吏?諛붿씠???湲??쒓컙??50ms ?댁빞 ?섏떊 泥섎━??


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

  return (float)((float)windData / 10.0);//?랁뼢? 10諛???媛믪씠 ?섏떊??
  //?랁뼢1234媛 ?섏떊 -> 123.4?꾩엫 ?곕씪???쒕씪?대쾭??媛믪쓽 ?⑥쐞???꾩엫, ?곕씪??10?쇰줈 ?섎늿媛믪쓣 由ы꽩??

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
 * 怨좎젙???띾룄濡??ъ슜?섎뒗 ?쇱꽌?ㅼ? ?ы듃?ㅼ젙留?留ㅺ컻蹂?섎줈 諛쏆븘??泥섎━
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