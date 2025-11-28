#include "temperature\hj_temperature.h"

#include <string.h>
#include <math.h>

#include "app_sensor.h"
#include "modbus_master.h"
#include "temperature\temperature_define.h"
#include "config_sensor.h"
#include "temperature\hj_temperature_define.h"
#include "os_user_def.h"
#include "drv_rs485.h"
#include "drv_rs232.h"

typedef struct hj_huminity_cfg_s
{
  modbus_h_t modbus;
  void *sem;
} hj_huminity_cfg_t;

driver_t hjHumi_drv;
hj_huminity_cfg_t hj_huminity_cfg;

float hjHuminity_read(driver_t *driver, uint8_t *err);


temperature_api_t hjHumiApi = {
    .read = hjHuminity_read};

driver_t *hjHuminity_open(int32_t num, void *opt)
{

  hjtemp_config_t *hjtemp = opt;
  uart_config_t uart_config;


  if (hjHumi_drv.opened)
  {
    return &hjHumi_drv;
  }

  hjHumi_drv.opened = true;
  uart_config.baud = 9600;
  uart_config.dataLen = UART_DATA_LEN_8;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;

  hj_huminity_cfg.modbus.name = "hjhumi";
  
  switch (hjtemp->physical_layer)
  {
    case ePHYSICAL_RS232:
      hj_huminity_cfg.modbus.modebus_type = eMODBUS_RS232;
      hj_huminity_cfg.modbus.id = hjtemp->modbus_id;
      hj_huminity_cfg.modbus.port_num = uart_num_to_driver_num(hjtemp->rs232_port);
      drv_uart_init(hj_huminity_cfg.modbus.port_num,&uart_config,"Huminity");
      break;

    case ePHYSICAL_RS485:
      hj_huminity_cfg.modbus.modebus_type = eMODBUS_RS485;
      hj_huminity_cfg.modbus.id = hjtemp->modbus_id;
      hj_huminity_cfg.modbus.port_num = rs485_num_to_driver_num(hjtemp->rs485_port);
      drv_rs485_init(hj_huminity_cfg.modbus.port_num,&uart_config,"Huminity");
      break;
  }


  hjHumi_drv.api = &hjHumiApi;
  hjHumi_drv.cfg = &hj_huminity_cfg;


  OS_CREATE_BINARY_SEM(hj_huminity_cfg.sem);
  
  return &hjHumi_drv;
}


float hjHuminity_read(driver_t *driver, uint8_t *err)
{
  float temp;
  uint16_t reg[2];
  hj_huminity_cfg_t *cfg = driver->cfg;
  int32_t ret;

  ret = modbus_read_hold_reg(&cfg->modbus, HJ_REG_NUM_HUMI, reg, 1);

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

driver_t * hjHumi_opened(void)
{
  if(hjHumi_drv.opened)
  {
    return &hjHumi_drv;
  }

  return NULL;
}