#include "hj_temperature.h"

#include <string.h>
#include <math.h>

#include "app_sensor.h"
#include "modbus_master.h"
#include "temperature_define.h"
#include "config_sensor.h"
#include "hj_temperature_define.h"
#include "drv_rs232.h"
#include "drv_rs485.h"

typedef struct hj_temperature_cfg_s
{
  modbus_h_t modbus;
} hj_temperature_cfg_t;

driver_t hjTemp_drv;
hj_temperature_cfg_t hj_temperature_cfg;

float hjTemperature_read(driver_t *driver, uint8_t *err);


temperature_api_t hjTempApi = {
    .read = hjTemperature_read};

driver_t *hjTemperature_open(int32_t num, void *opt)
{
  uart_config_t uart_config;
  hjtemp_config_t *hjtemp = opt;


  if (hjTemp_drv.opened)
  { 
    return &hjTemp_drv;
  }

  hj_temperature_cfg.modbus.id = hjtemp->modbus_id;

  uart_config.baud = 9600;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;
  uart_config.dataLen = UART_DATA_LEN_8;

  switch (hjtemp->physical_layer)
  {
    case ePHYSICAL_RS232:
      hj_temperature_cfg.modbus.port_num = uart_num_to_driver_num(hjtemp->rs232_port);
      hj_temperature_cfg.modbus.modebus_type = eMODBUS_RS232;
      drv_rs232_init(hj_temperature_cfg.modbus.port_num,&uart_config);
      break;
    case ePHYSICAL_RS485:
      hj_temperature_cfg.modbus.port_num = rs485_num_to_driver_num(hjtemp->rs485_port);
      hj_temperature_cfg.modbus.modebus_type = eMODBUS_RS485;
      drv_rs485_init(hj_temperature_cfg.modbus.port_num, &uart_config);
      break;
  }

  hjTemp_drv.opened = true;
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

  ret = modbus_read_hold_reg(&cfg->modbus, cfg->modbus.id, HJ_REG_NUM_TEMP, reg, 1);

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




void hjtemperature_ctrl(driver_t *driver, eHJTEMPERATURE_OPT_t ctrl, void *w_opt, void *r_opt,
                        uint8_t *err)
{
  int32_t ret = 0;
  hj_temperature_cfg_t *cfg = driver->cfg;
uint16_t data;
  

  switch (ctrl)
  {
    case eTEMP_SET_OFFSET:
      data = *(int32_t *)w_opt;
      modbus_write_single_reg(&cfg->modbus, cfg->modbus.id, HJ_REG_NUM_TEMP_OFFSET, data);
      break;
    case eHUMI_SET_OFFSET:
      data = *(int32_t *)w_opt;
      modbus_write_single_reg(&cfg->modbus, cfg->modbus.id, HJ_REG_NUM_HUMI_OFFSET, data);
      break;
    case eTEMP_GET_OFFSET:
      ret = modbus_read_hold_reg(&cfg->modbus, cfg->modbus.id, HJ_REG_NUM_TEMP_OFFSET,
                                          &data, 1);
      *err = ret;
      if (ret == 0)
      {
        
        *((int32_t *)r_opt) = (int16_t)data;
      }
      break;
    case eHUMI_GET_OFFSET:
      ret = modbus_read_hold_reg(&cfg->modbus, cfg->modbus.id, HJ_REG_NUM_HUMI_OFFSET,
                                          &data, 1);
      *err = ret;
      if (ret == 0)
      {
        *((int32_t *)r_opt) = (int16_t)data;
      }
      break;
  }
}

driver_t *hjtemp_opened(void)
{
  if(hjTemp_drv.opened)
  {
    return &hjTemp_drv;
  }

  return NULL;
}


modbus_h_t* get_hjtemperature_bus_io(void)
{
  hj_temperature_cfg_t *cfg;
  driver_t *driver;

  driver = hjtemp_opened();
  if(driver)
  {
    cfg = driver->cfg;
    return &cfg->modbus;
  }

    return NULL;
}