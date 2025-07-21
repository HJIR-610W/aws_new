

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

solarRadiation_api_t g_ott_smp3_api = {.read = smp3_solar_read,.set=NULL,.get =NULL};


void ott_smp3_initialize(void)
{
  
  modbus_write_single_coil(&g_ott_smp3_cfg.modbus, COIL_IO_CLEAR_ERROR, true);
}

driver_t *ott_smp3_open(int32_t num, void *opt)
{
  uart_config_t uart_config;
  ott_smp3_config_t *ott = (ott_smp3_config_t *)opt;

  uart_config.baud = 19200;
  uart_config.parityIdx = PARITY_EVEN;
  uart_config.stop_bit = UART_STOP_BIT_1;
  uart_config.dataLen = UART_DATA_LEN_8;

  g_ott_smp3_cfg.modbus.modebus_type = eMODBUS_RS485;
  g_ott_smp3_cfg.modbus.port_num = rs485_num_to_driver_num(ott->port);
  g_ott_smp3_cfg.modbus.id = ott->modbus_id;
  drv_rs485_init(g_ott_smp3_cfg.modbus.port_num,&uart_config);

  g_ott_smp3_driver.cfg = &g_ott_smp3_cfg;
  g_ott_smp3_driver.api = &g_ott_smp3_api;
  g_ott_smp3_driver.opened = true;
  
  ott_smp3_initialize();

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

/*
요청 01 04 00 1A 00 02 50 0C     Input Registers 읽기 주소 26, 2개 읽기
응답 01 04 04 00 09 00 00 2B 86
요청 01 04 00 00 00 14 F0 05   Input Registers 20개 읽기 주소 0
응답 01 04 28 02 59 00 64 00 01 00 08 00 01 00 F4 00 F2 00 00 01 20 00 7A 00 00 00 00 00 00 00 00 00 00 00 00 03 EF 00 03 00 00 D3 0C BA AB
요청 01 02 00 02 00 08 D8 0C   Read Discrete Inputs  2번 주소
응답 01 02 01 08 A0 4E


파싱
Input Registers 20개 읽기 주소 0
| 주소 (Offset) | 값 (Hex) | 값 (10진수) |
| ----------- | ------- | -------- |
| `0x0000`    | `02 59` | `601`    | IO_DEIVCE_TYPE
| `0x0001`    | `00 64` | `100`    |
| `0x0002`    | `00 01` | `1`      |IO_OPERATIONAL_MODE,Normal Mode
| `0x0003`    | `00 08` | `8`      |IO_STATUS_FLAGS,Error flag 0x08 갑작스러 전원 리셋
| `0x0004`    | `00 01` | `1`      |IO_SCALE_FACTOR,1 floating point result = integer register X / 10
| `0x0005`    | `00 F4` | `244`    |IO_SENSOR1_DATA
| `0x0006`    | `00 F2` | `242`    |IO_RAW_SENSOR1_DATA
| `0x0007`    | `00 00` | `0`      |IO_STDEV_SENSOR1
| `0x0008`    | `01 20` | `288`    |IO_BODY_TEMPERATURE
| `0x0009`    | `00 7A` | `122`    |IO_EXT_POWER_SENSOR
| `0x000A`    | `00 00` | `0`      |
| `0x000B`    | `00 00` | `0`      |
| `0x000C`    | `00 00` | `0`      |
| `0x000D`    | `00 00` | `0`      |
| `0x000E`    | `00 00` | `0`      |
| `0x000F`    | `00 00` | `0`      |IO_TILT
| `0x0010`    | `03 EF` | `1007`   |IO_RH  /10
| `0x0011`    | `00 03` | `3`      |
| `0x0012`    | `00 00` | `0`      |


Write Single Coil(COIL_IO_CLEAR_ERROR 지우기)
01 05 00 0A FF 00 AC 38
01 05 00 0A FF 00 AC 38
*/

float ott_cvt_scale_factor(int32_t scale_factor)
{
  switch (scale_factor)
  {
    case 2:
      return 100;
    case 1:
      return 10;
    case 0:
    return 1;
    case -1:
    return 0.1;
    default:
      return 1;
  }
}

float smp3_solar_read(driver_t *driver, uint8_t *err)
{
  uint16_t reg[10];
  int32_t ret;
  float scale_factor;
  float solar_radiation;
  ott_smp3_cfg_t *cfg = driver->cfg; 


  ret = modbus_read_input_reg(&cfg->modbus,  REG_IO_DEVICE_TYPE, reg, _countof(reg));

  if(ret)
  {
    *err = DRV_ERR_TIMEOUT;
    solar_radiation = NAN;
  }
  else
  {
    *err = DRV_ERR_NONE;
    memcpy(&g_ott_smp3_system.status, &reg[REG_IO_STATUS_FLAGS], 2);
    //print_ott();
    scale_factor = ott_cvt_scale_factor(reg[REG_IO_SCALE_FACTOR]);

    //temp = modbus_regs_to_float(reg[3], reg[4]);
    solar_radiation = (float)reg[REG_IO_SENSOR1_DATA] / scale_factor;
  }

  return solar_radiation;
}

