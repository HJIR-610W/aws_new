

#include "Sensors\wind_speed\hj_wind.h"


#include <math.h>
#include <string.h>

#include "drv_rs485.h"
#include "app_sensor.h"
#include "cmsis_os2.h"
#include "debug_io.h"
#include "drv_rs485.h"
#include "drv_rs232.h"
#include "util_memory.h"
#include "config_sensor.h"

typedef struct hjwind_cfg_s
{
  int32_t  rs485_num;
} hjwind_cfg_t;


void set_hjwind_direction(void *handle, wind_set_option_t option, void *value);
float read_hjwind_direction(void *driver, uint8_t type, uint8_t *err);
void set_hjwind_direction(void *handle, wind_set_option_t option, void *value);


// 02 01 02 01 01 05 03 
// 02 02 02 01 01 06 03
uint16_t make_hjwind_direction(uint8_t *sSend, uint8_t id)
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

bool is_hjwin_direction(uint8_t *frame, uint16_t len)
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



float read_hjwind_direction(void *driver, uint8_t channel, uint8_t *err)
{
  uint8_t send[10];
  uint8_t recv[10];
  uint16_t len;
  uint16_t windData = 0;
  hjwind_cfg_t *cfg = ((driver_t *)driver)->cfg;

  osDelay(30);
  len = make_hjwind_direction(send, channel);

  drv_rs485_flush_rx(cfg->rs485_num);
  drv_rs485_send(cfg->rs485_num, send, len);
   

  len = drv_rs485_recv_opt(cfg->rs485_num, recv, sizeof(recv), 50,5); 

  if(len == 0)
  {
    *err = DRV_ERR_TIMEOUT;
    return NAN;
  }

  if (len && is_hjwin_direction(recv, len))
  {
    if (channel == HJ_WIND_CHANNEL_DIRECTION)
    {
      if (((recv[1] & 0x7f) == 0x02))
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


void get_hjwind_direction(driver_t *drv, int cmd, void *val)
{

}

void set_hjwind_direction(void *handle, wind_set_option_t option, void *value)
{


}

wind_api_t hjwind_direction_api = {.read = read_hjwind_direction, .set = set_hjwind_direction};
hjwind_cfg_t g_hjwind_direction_cfg;
driver_t g_hjwind_direction_driver;

driver_t *hjwind_direction_open(uint8_t num, void *opt)
{
  uart_config_t uart_config;
  wind_speed_hj_pulse_config_t *hjwind_config = opt;

  (void)num;

  if (g_hjwind_direction_driver.opened == true)
  {
    return &g_hjwind_direction_driver;
  }

  uart_config.baud = 9600;
  uart_config.parity_index = 0;
  uart_config.dataLen = 8;
  uart_config.stop_bit = 1;

  g_hjwind_direction_cfg.rs485_num = rs485_num_to_driver_num(hjwind_config->rs485_port);
  drv_rs485_init((int)g_hjwind_direction_cfg.rs485_num, &uart_config,"Wind Direction");

  g_hjwind_direction_driver.name = "hj_wind";
  g_hjwind_direction_driver.cfg = &g_hjwind_direction_cfg;
  g_hjwind_direction_driver.api = &hjwind_direction_api;

  g_hjwind_direction_driver.opened = true;

  return &g_hjwind_direction_driver;
}