

#include <string.h>

#include "cmsis_os2.h"
#include "config_app.h"
#include "dev_io.h"
#include "pcb_define.h"
#include "driver_uart.h"
#include "driver_di.h"
#include "driver_do.h"




driver_t *g_ble_drv;
driver_t *g_btm_power;
driver_t *g_btm_status;

const osThreadAttr_t bleTask_attributes = {
  .name = "bleTask",
  .stack_size = 1024,
  .priority = (osPriority_t) osPriorityNormal1,
};


void ble_cmd(const char *cmd)
{
  uint8_t buff[100];
  int32_t len;
  

    driver_uart_send(g_ble_drv,(uint8_t*)cmd,strlen(cmd));
    len = driver_uart_recv(g_ble_drv,buff,sizeof(buff),1000);
    if(len)
    {
      LOG_MEM(buff,len,0,16);
    }
}

void bleTask(void *arg)
{
#if 0 
  uint8_t buff[10];
  int32_t len;
  uint8_t temp[10];
#endif
  while(1)
  {
#if 0 
    ble_cmd("ATZ\r");
      osDelay(1000);

 
    ble_cmd("AT+INFO?\r");
    osDelay(1000);  
    ble_cmd("AT+ADVOFF\r");
    osDelay(1000);  
    ble_cmd("AT+ADVON\r");
    while(1)
    {
      len = driver_uart_recv(g_ble_drv,buff,sizeof(buff),1000);
      if(len)
      {
        memcpy(temp,buff,len);
        temp[0]='r';
        temp[len]=0;
        ble_cmd(temp);
        LOG_MEM(buff,len,0,16);
      }
    }
#endif
    osDelay(1000);
  }
}

  
  
void bleTask_init(void)
{
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};


  uart_config.baud = 9600;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;
  
  g_ble_drv = driver_uart_open(UART_1_TTL,&uart_config);

  g_btm_power  = driver_do_open(DO_BTM_PWCTRL,0);
  g_btm_status = driver_di_open(DI_BTM_STATUS,0);

  
  driver_do_high(g_btm_power);

  osThreadNew(bleTask, NULL, &bleTask_attributes);

}