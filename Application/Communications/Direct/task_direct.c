#include "aws_protocol.h"
#include "app_rtc.h"
#include "cmsis_os2.h"
#include "config.h"
#include "task_isrEvent.h"
#include "dev_io.h"
#include "driver_uart.h"


const osThreadAttr_t directTask_attributes = {
  .name = "directTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityNormal,
};


driver_t *direct_driver;
 
void directTask(void *arg)
{
  uint8_t rx_buff[100];
  uint8_t tx_buff[512];
  uint32_t startTime;
  int32_t len;
uart_optTimeOut_t opt;
  
  opt.frameTimeOutMs=10000;
  opt.dataTimeOutMs = 10;

  startTime = osKernelGetTickCount();

  while(1)
  {
    len = driver_uart_recv_opt(direct_driver,rx_buff,sizeof(rx_buff),eUART_OPT_DATA_TIMEOUT_1,10);
    if(len)
    {
      System.direct_link_status = LINK_UP;
      update_cnt(&System.direct_rx_cnt);
      len = aws_cmd(rx_buff,len,tx_buff,sizeof(tx_buff),0);
      if(len)
      {
        driver_uart_send(direct_driver,tx_buff,len);
        update_cnt(&System.direct_tx_cnt);
      }
      startTime  = osKernelGetTickCount();
    }

    if((osKernelGetTickCount()-startTime)>3600000)
    {
      System.direct_link_status = LINK_DOWN;
      startTime  = osKernelGetTickCount();
    }
  }
}


void directTask_init(void)
{
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};
  
  System.direct_link_status = STATUS_IDLE;
  System.direct_tx_cnt=0;
  System.direct_rx_cnt=0;


  uart_config.baud = config.direct_baud;
  uart_config.parityIdx = 0;
  uart_config.stop_bit = 0;

  direct_driver = driver_uart_open(UART_8_CDMA,&uart_config);

  osThreadNew(directTask, NULL, &directTask_attributes);
}