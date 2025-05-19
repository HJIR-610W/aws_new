#include "task_direct.h"
#include "kma_protocol_handler.h"
#include "app_rtc.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "dev_io.h"
#include "driver_uart.h"
#include "task_isrEvent.h"


const osThreadAttr_t directTask_attributes = {
    .name = "directTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityNormal,
};


static direct_status_t g_direct_system;
driver_t *direct_driver;

direct_status_t *get_direct_system(void)
{ 
  return &g_direct_system; 
}

void directTask(void *arg)
{
  uint8_t rx_buff[100];
  uint8_t tx_buffer[KMA_TX_BUFFER_SIZE];
  uint32_t startTime;
  int32_t len;
uart_optTimeOut_t opt;
  
  opt.frameTimeOutMs=10000;
  opt.dataTimeOutMs = 10;

  startTime = osKernelGetTickCount();

  while(1)
  {
    len = driver_uart_recv_opt(direct_driver,rx_buff,sizeof(rx_buff),10000,10);
    if(len)
    {
      g_direct_system.link_status = eDIRECT_LINK_UP;

      UPDATE_CNT(g_direct_system.rx_cnt, 99);
      len = kma_cmd_handler(rx_buff, len, tx_buffer, eREQ_SOURCE_DIRECT);
      if(len)
      {
        driver_uart_send(direct_driver, tx_buffer, len);
        UPDATE_CNT(g_direct_system.tx_cnt, 99);
      }
      startTime  = osKernelGetTickCount();
    }

    if((osKernelGetTickCount()-startTime)>3600000)
    {
      g_direct_system.link_status = eDIRECT_LINK_DOWN;
      startTime  = osKernelGetTickCount();
    }
  }
}


void directTask_init(void)
{
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  g_direct_system.link_status = eDIRECT_LINK_IDLE;
  g_direct_system.tx_cnt = 0;
  g_direct_system.rx_cnt = 0;

  uart_config.baud = get_config_app()->direct_baud;
  uart_config.parityIdx = 0;
  uart_config.stop_bit = 0;

  direct_driver = driver_uart_open(UART_8_CDMA,&uart_config);

  osThreadNew(directTask, NULL, &directTask_attributes);
}