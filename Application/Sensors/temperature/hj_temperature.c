#include "hj_temperature.h"

#include <string.h>
#include <math.h>

#include "app_sensor.h"
#include "driver_modbus.h"
#include "temperature_define.h"
#include "config_sensor.h"
#include "hj_temperature_define.h"
#include "app_rs232.h"
typedef struct hj_temperature_cfg_s
{
  driver_t *bus_io;
} hj_temperature_cfg_t;

driver_t hjTemp_drv;
hj_temperature_cfg_t hj_temperature_cfg;

float hjTemperature_read(driver_t *driver, uint8_t *err);
void hjTemperature_set(driver_t *handle, temperature_set_option_t option, void *value);
int32_t hjTemperature_get(driver_t *driver, temperature_get_option_t option, void *value);

temperature_api_t hjTempApi = {
    .read = hjTemperature_read, .set = hjTemperature_set, .get = hjTemperature_get};

driver_t *hjTemperature_open(int32_t num, void *opt)
{
  modbus_init_t modbus_init;
  hjtemp_config_t *hjtemp = opt;


  int32_t port = hjtemp->port;



  if (hjTemp_drv.opened) { return &hjTemp_drv; }

  modbus_init.baud = 9600;
  modbus_init.parityIdx = 0;
  modbus_init.stop = 1;


  switch (hjtemp->physical_layer)
  {
    case ePHYSICAL_RS232:
      modbus_init.port_num = uart_num_to_driver_num(port);
      hj_temperature_cfg.bus_io =
          driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_232, &modbus_init);
      break;
    case ePHYSICAL_RS485:
      modbus_init.port_num = port;
      hj_temperature_cfg.bus_io =
          driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_485, &modbus_init);
      break;
  }



  hjTemp_drv.api = &hjTempApi;
  hjTemp_drv.cfg = &hj_temperature_cfg;

  return &hjTemp_drv;
}

float get_float(uint16_t high, uint16_t low)
{
  uint32_t temp = 0;
  float a;

  temp = high << 16 | low;
  memcpy(&a, &temp, 4);

  return a;
}

float hjTemperature_read(driver_t *driver, uint8_t *err)
{
  float temp;
  uint16_t reg[2];
  hj_temperature_cfg_t *cfg = driver->cfg;
  int32_t ret;

  ret = driver_modbus_m_read_multi_reg(cfg->bus_io, 1, HJ_REG_NUM_TEMP, reg, 2);

  if(ret)
  {
    *err = DRV_ERR_TIMEOUT;
    temp = NAN;
  }
  else
  {
    *err = DRV_ERR_NONE;
    temp = ((float)reg[0])/100.0f;
  }

  return temp;
}

void hjTemperature_set(driver_t *driver, temperature_set_option_t option, void *value)
{
  hj_temperature_cfg_t *cfg = driver->cfg;

  uint16_t data = (uint16_t )value;

  switch (option)
  {
    case eTEMP_SET_OFFSET:
      driver_modbus_m_write_single_reg(cfg->bus_io, 1, HJ_REG_NUM_TEMP_OFFSET,data);
      break;
      case eHUMI_SET_OFFSET:
      driver_modbus_m_write_single_reg(cfg->bus_io, 1, HJ_REG_NUM_HUMI_OFFSET,data);
      break;
    default:
      break;
  }
}

int32_t hjTemperature_get(driver_t *driver, temperature_get_option_t option, void *value)
{
  hj_temperature_cfg_t *cfg = driver->cfg;
  int32_t ret=0;
  uint16_t data = (uint16_t)value;

  switch (option)
  {
    case eTEMP_GET_OFFSET:
      ret = driver_modbus_m_read_multi_reg(cfg->bus_io, 1, HJ_REG_NUM_TEMP_OFFSET, &data, 1);
      *((uint16_t *)value) = data;
      break;
      case eHUMI_GET_OFFSET:
      ret = driver_modbus_m_read_multi_reg(cfg->bus_io, 1, HJ_REG_NUM_HUMI_OFFSET, &data, 1);
      *((uint16_t *)value) = data;
      break;
    default:
      break;
  }

  return ret;
}