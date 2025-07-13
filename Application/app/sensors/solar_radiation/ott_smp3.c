

#include "ott_smp3.h"

#include <math.h>
#include <string.h>
#include "app_sensor.h"
#include "config_sensor.h"
#include "driver_modbus.h"
#include "ott_smp3_define.h"

#include "dev_io.h"


typedef struct ott_smp3_cfg_s
{
  driver_t *bus_io;
  uint8_t modbus_id;

} ott_smp3_cfg_t;


typedef struct ott_smp3_system_s
{
  status_flags_t status;
} ott_smp3_system_t;

ott_smp3_system_t g_ott_smp3_system;
driver_t g_ott_smp3_driver;
ott_smp3_cfg_t g_ott_smp3_cfg;
void ott_smp3_set(driver_t *handle, solarRadiation_set_option_t option, void *value);
void ott_smp3_get(driver_t *handle, solarRadiation_get_option_t option, void *value);

    float smp3_solar_read(driver_t *driver, uint8_t *err);

solarRadiation_api_t g_ott_smp3_api = {.read = smp3_solar_read,.set=ott_smp3_set,.get =ott_smp3_get};

driver_t *ott_smp3_open(int32_t num, void *opt)
{
  modbus_init_t modbus_init;
  ott_smp3_config_t *ott = (ott_smp3_config_t *)opt;

  modbus_init.baud = 9600;
  modbus_init.parityIdx = 0;
  modbus_init.stop = 1;
  modbus_init.port_num = ott->port;
  ;
  g_ott_smp3_cfg.modbus_id = ott->modbus_id;
  g_ott_smp3_cfg.bus_io = driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_485, &modbus_init);
  
  g_ott_smp3_driver.cfg = &g_ott_smp3_cfg;
  g_ott_smp3_driver.api = &g_ott_smp3_api;

      return &g_ott_smp3_driver;
}

float modbus_regs_to_float(uint16_t msb, uint16_t lsb)
{
  uint32_t raw = ((uint32_t)msb << 16) | lsb;
  float result;
  memcpy(&result, &raw, sizeof(result));
  return result;
}


void print_ott(void)
{
  uint16_t status=0;
  
  memcpy(&status,&g_ott_smp3_system.status,2);
  if(status)
  {
    io_printf("Signal quality error:       %d\r\n", g_ott_smp3_system.status.signal_quality);
  io_printf("Overflow error:             %d\r\n", g_ott_smp3_system.status.overflow_error);
  io_printf("Underflow error:            %d\r\n", g_ott_smp3_system.status.underflow_error);
  io_printf("General error:              %d\r\n", g_ott_smp3_system.status.general_error);
  io_printf("ADC error:                  %d\r\n", g_ott_smp3_system.status.adc_error);
  io_printf("DAC error:                  %d\r\n", g_ott_smp3_system.status.dac_error);
  io_printf("Calibration error:          %d\r\n", g_ott_smp3_system.status.calibration_error);
  io_printf("EEPROM update error:        %d\r\n", g_ott_smp3_system.status.eeprom_update_error);
  io_printf("Power failure error:        %d\r\n", g_ott_smp3_system.status.power_failure_error);
  io_printf("Tilt sensor error:          %d\r\n", g_ott_smp3_system.status.tilt_sensor_error);
  io_printf("RH sensor error:            %d\r\n", g_ott_smp3_system.status.rh_sensor_error);
  io_printf("RH threshold warning:       %d\r\n", g_ott_smp3_system.status.rh_threshold_warning);
  io_printf("Body temperature error:     %d\r\n", g_ott_smp3_system.status.body_temp_error);
  }
}

float smp3_solar_read(driver_t *driver, uint8_t *err)
{
  float temp;
  uint16_t reg[5];
  ott_smp3_cfg_t *cfg = driver->cfg;
  int32_t ret;

  
  ret = driver_modbus_m_read_hold_reg(cfg->bus_io, cfg->modbus_id, REG_U_STATUS_FLAGS, reg, _countof(reg));

  if(ret)
  {
    *err = DRV_ERR_TIMEOUT;
    temp = NAN;
  }
  else
  {
    *err = DRV_ERR_NONE;
    memcpy(&g_ott_smp3_system.status,&reg[0],2);
    //print_ott();
    temp = modbus_regs_to_float(reg[3], reg[4]);
  }

  return temp;
}

void ott_smp3_set(driver_t *handle, solarRadiation_set_option_t option, void *value)
{

}

void ott_smp3_get(driver_t *handle, solarRadiation_get_option_t option, void *value)
{

}