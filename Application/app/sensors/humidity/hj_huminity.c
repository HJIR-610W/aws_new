#include "temperature\hj_temperature.h"

#include <string.h>
#include <math.h>

#include "app_sensor.h"
#include "driver_modbus.h"
#include "temperature\temperature_define.h"
#include "config_sensor.h"
#include "temperature\hj_temperature_define.h"
#include "os_user_def.h"
typedef struct hj_huminity_cfg_s
{
  driver_t *bus_io;
  void *sem;
} hj_huminity_cfg_t;

driver_t hjHumi_drv;
hj_huminity_cfg_t hj_huminity_cfg;

float hjHuminity_read(driver_t *driver, uint8_t *err);


temperature_api_t hjHumiApi = {
    .read = hjHuminity_read};

driver_t *hjHuminity_open(int32_t num, void *opt)
{
  modbus_init_t modbus_init;
  hjtemp_config_t *hjtemp = opt;

  int32_t port;

  if (hjHumi_drv.opened)
  {
    return &hjHumi_drv;
  }

  hjHumi_drv.opened = true;
  modbus_init.baud = 9600;
  modbus_init.parityIdx = 0;
  modbus_init.stop = 1;


  switch (hjtemp->physical_layer)
  {
    case ePHYSICAL_RS232:
      modbus_init.port_num =  uart_num_to_driver_num(hjtemp->rs232_port);
      hj_huminity_cfg.bus_io =
          driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_232, &modbus_init);
      break;
    case ePHYSICAL_RS485:
       modbus_init.port_num  = hjtemp->rs485_port;
      hj_huminity_cfg.bus_io =
          driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_485, &modbus_init);
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

driver_t * hjHumi_opened(void)
{
  if(hjHumi_drv.opened)
  {
    return &hjHumi_drv;
  }

  return NULL;
}