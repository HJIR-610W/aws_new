
#include "ls1024.h"

#include <string.h>
#include <math.h>
#include "cmsis_os2.h"
#include "drv_rs485.h"
#include "drv_rs232.h"
#include "pcb_define.h"
#include "util_memory.h"
#include "util_time.h"
#include "os_user_def.h"
#include "driver_modbus.h"



void ls1024_read(driver_t *driver, charger_data_t *charger_data,
                 uint8_t *err) ;
charger_api_t ls1024_api = {.read = ls1024_read};

typedef struct ls1024_cfg_s
{
  driver_t *bus_io;
}ls1024_cfg_t;

driver_t ls1024_driver;
ls1024_cfg_t ls1024_cfg;



driver_t *ls1024_open(int32_t num,void *opt)
{
  modbus_init_t modbus_init;

  if(ls1024_driver.opened)
  {
    return &ls1024_driver;
  }

  modbus_init.baud = 115200;
  modbus_init.parityIdx = 0;
  modbus_init.stop = 1;

  modbus_init.port_num = RS485_B;
  ls1024_cfg.bus_io = driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_485, &modbus_init);

  ls1024_driver.cfg = &ls1024_cfg;
  ls1024_driver.api = &ls1024_api;

  OS_CREATE_BINARY_SEM(ls1024_driver.sem);

  ls1024_driver.opened =true;
  
  return &ls1024_driver;
}

void ls1024_read(driver_t *driver, charger_data_t *charger_data, uint8_t *err)
{

  uint16_t reg[15];
  ls1024_cfg_t *cfg = driver->cfg;
  int32_t ret;

  ret = driver_modbus_m_read_input_reg(cfg->bus_io, 1, 0x3100, reg, 15);

  if (ret)
  {
    *err = DRV_ERR_TIMEOUT;
  }
  else
  {
    *err = DRV_ERR_NONE;
    // Solar (PV1)
    charger_data->solar1Volt    = (float)reg[0] / 100.0f;  // 0x3100
    charger_data->solar1Current = (float)reg[1] / 100.0f;  // 0x3101

    // Battery1
    charger_data->battery1      = (float)reg[4] / 100.0f;  // 0x3104

    // Load1
    charger_data->load1Current  = (float)reg[13] / 100.0f; // 0x310D

    // Unused (추가 솔라 및 배터리는 현재 미사용)
    charger_data->solar2Volt    = 0.0f;
    charger_data->solar2Current = 0.0f;
    charger_data->battery2      = 0.0f;
    charger_data->load2Current  = 0.0f;
    charger_data->load3Current  = 0.0f;

  }

}