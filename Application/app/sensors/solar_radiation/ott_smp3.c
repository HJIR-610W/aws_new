

#include "ott_smp3.h"

#include <math.h>
#include <string.h>
#include "app_sensor.h"
#include "config_sensor.h"
#include "modbus_master.h"
#include "ott_smp3_define.h"
#include "dev_io.h"
#include "drv_rs485.h"

typedef struct ott_smp3_cfg_s
{
  modbus_h_t modbus;
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
  uart_config_t uart_config;
  ott_smp3_config_t *ott = (ott_smp3_config_t *)opt;

  uart_config.baud = 9600;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;
  uart_config.dataLen = UART_DATA_LEN_8;

  g_ott_smp3_cfg.modbus.modebus_type = eMODBUS_RS485;
  g_ott_smp3_cfg.modbus.port_num = ott->port;

  drv_rs485_init(g_ott_smp3_cfg.modbus.port_num,&uart_config);

  g_ott_smp3_driver.cfg = &g_ott_smp3_cfg;
  g_ott_smp3_driver.opened = true;
  
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

  
  ret = modbus_read_hold_reg(&cfg->modbus, cfg->modbus_id, REG_U_STATUS_FLAGS, reg, _countof(reg));

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