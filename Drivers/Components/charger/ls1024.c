
#include <string.h>

#include "cmsis_os2.h"

#include "pcb_define.h"
#include "ls1024.h"
#include "driver_uart.h"
#include "utile_time.h"
#include "utile.h"





int32_t ls1024_read(driver_t *chg,charger_data_t *data,uint8_t *err);


charger_api_t ls1024_api ={.read = ls1024_read};

typedef struct ls1024_cfg_s
{
  driver_t *rs232_io;
}ls1024_cfg_t;

driver_t ls1024_driver;
ls1024_cfg_t ls1024_cfg;


driver_t *ls1024_open(int32_t num,void *opt)
{
  uart_config_t uart_config;

  if(ls1024_driver.opened)
  {
    return &ls1024_driver;
  }

    uart_config.baud = 57600;
    uart_config.dataLen   = 8;
    uart_config.parityIdx = 0;
    uart_config.stop_bit  = 1;

    ls1024_driver.opened = true;
    ls1024_driver.api = &ls1024_api;
    ls1024_cfg.rs232_io = driver_uart_open(UART_0_D_SUB_0,&uart_config);

    ls1024_driver.cfg = &ls1024_cfg;

#if FREE_RTOS_USE
  if(ls1024_driver.sem == NULL)
  {
    ls1024_driver.sem = osSemaphoreNew(1, 1, NULL); 
  }
#endif



  return &ls1024_driver;
}


int32_t ls1024_read(driver_t *chg,charger_data_t *charger_data,uint8_t *err)
{

}