#include "hj_temperature.h"

#include <string.h>

#include "app_sensor.h"
#include "driver_modbus.h"
#include "temperature_define.h"

typedef struct hj_temperature_cfg_s
{
  driver_t *bus_io;
} hj_temperature_cfg_t;

driver_t hjTemp_drv;
hj_temperature_cfg_t hj_temperature_cfg;

float hjTemperature_read(driver_t *driver, uint8_t *err);
void hjTemperature_set(driver_t *handle, temperature_set_option_t option, void *value);

temperature_api_t hjTempApi = {.read = hjTemperature_read, .set = hjTemperature_set};

driver_t *hjTemperature_open(int32_t num, void *opt)
{
  modbus_init_t modbus_init;
  hjtemp_config_t *hjtemp = opt;

  int32_t port = hjtemp->rs485_port;

  if (hjTemp_drv.opened)
  {
    return &hjTemp_drv;
  }

  modbus_init.baud = 9600;
  modbus_init.parityIdx = 0;
  modbus_init.stop = 1;
  modbus_init.port_num = port;
  hj_temperature_cfg.bus_io =
      driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_485, &modbus_init);

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

  driver_modbus_m_read_multi_reg(cfg->bus_io, 0, 0, reg, 2);

  temp = get_float(reg[0], reg[1]);

  return temp;
}

void hjTemperature_set(driver_t *handle, temperature_set_option_t option, void *value) {}