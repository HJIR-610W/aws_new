#include "temperature\hj_temperature.h"

#include <string.h>
#include <math.h>

#include "app_sensor.h"
#include "driver_modbus.h"
#include "temperature\temperature_define.h"
#include "config_sensor.h"
#include "temperature\hj_temperature_define.h"

typedef struct hj_huminity_cfg_s
{
  driver_t *bus_io;
} hj_huminity_cfg_t;

driver_t hjHumi_drv;
hj_huminity_cfg_t hj_huminity_cfg;

float hjHuminity_read(driver_t *driver, uint8_t *err);
void hjHuminity_set(driver_t *driver, temperature_set_option_t option, void *value);
int32_t hjHuminity_get(driver_t *driver, temperature_get_option_t option, void *value);

temperature_api_t hjHumiApi = {
    .read = hjHuminity_read, .set = hjHuminity_set, .get = hjHuminity_get};

driver_t *hjHuminity_open(int32_t num, void *opt)
{
  modbus_init_t modbus_init;
  hjtemp_config_t *hjtemp = opt;

  int32_t port = hjtemp->port;

  if (hjHumi_drv.opened)
  {
    return &hjHumi_drv;
  }

  modbus_init.baud = 9600;
  modbus_init.parityIdx = 0;
  modbus_init.stop = 1;


  switch (hjtemp->physical_layer)
  {
    case ePHYSICAL_RS232:
      modbus_init.port_num =  uart_num_to_driver_num(port);
      hj_huminity_cfg.bus_io =
          driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_232, &modbus_init);
      break;
    case ePHYSICAL_RS485:
      hj_huminity_cfg.bus_io =
          driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_485, &modbus_init);
      break; 
  }


  hjHumi_drv.api = &hjHumiApi;
  hjHumi_drv.cfg = &hj_huminity_cfg;

  return &hjHumi_drv;
}


float hjHuminity_read(driver_t *driver, uint8_t *err)
{
  float temp;
  uint16_t reg[2];
  hj_huminity_cfg_t *cfg = driver->cfg;
  int32_t ret;

  ret = driver_modbus_m_read_hold_reg(cfg->bus_io, 1, HJ_REG_NUM_HUMI, reg, 1);

  if(ret)
  {
    *err = DRV_ERR_TIMEOUT;
    temp = NAN;
  }
  else
  {
    *err = 0;
    temp = (float)((float)reg[0]/100.0);
  }

  return temp;
}

void hjHuminity_set(driver_t *driver, temperature_set_option_t option, void *value)
{
  hj_huminity_cfg_t *cfg = driver->cfg;

  uint16_t data = (uint16_t)value;

  switch (option)
  {
    case eHUMI_SET_OFFSET:
      driver_modbus_m_write_single_reg(cfg->bus_io, 1, HJ_REG_NUM_HUMI_OFFSET, data);
      break;

    default:
      break;
  }
}

int32_t hjHuminity_get(driver_t *driver, temperature_get_option_t option, void *value)
{
  hj_huminity_cfg_t *cfg = driver->cfg;
  int32_t ret = 0;
  uint16_t data = (uint16_t)value;

  switch (option)
  {
    case eHUMI_GET_OFFSET:
      ret = driver_modbus_m_read_hold_reg(cfg->bus_io, 1, HJ_REG_NUM_HUMI_OFFSET, &data, 1);
      *((uint16_t *)value) = data;
      break;

    default:
      break;
  }

  return ret;
}