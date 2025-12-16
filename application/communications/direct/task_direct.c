#include "task_direct.h"
#include "kma_protocol_handler.h"
#include "bsp.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "debug_io.h"
#include "drv_rs232.h"
#include "task_event.h"
#include "update_fw.h"
#include "system_err.h"
#include "FreeRTOS.h"
#define DIRECT_TIMEOUT_MS 600000

const osThreadAttr_t directTask_attributes = {
    .name = "direct",
    .stack_size = TASK_STACK(TASK_DIRECT_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_DIRECT_DEF),
};

static direct_system_t g_direct_system;
static int32_t g_direct_uart_num;

direct_system_t *get_direct_system(void)
{ 
  return &g_direct_system; 
}


void directTask(void *arg)
{
  uint8_t rx_buff[512];
  uint8_t tx_buffer[KMA_TX_BUFFER_SIZE];
  uint32_t startTime;
  int32_t len;

  startTime = osKernelGetTickCount();

  g_direct_system.linkdown_remain_ms = DIRECT_TIMEOUT_MS;
  
  while (1)
  {
    len = drv_uart_recv_opt(g_direct_uart_num,rx_buff,sizeof(rx_buff),10000,10);
    if(len)
    {
      g_direct_system.link_status = eDIRECT_LINK_UP;
      g_direct_system.last_recv_time = time_timestamp();
      UPDATE_CNT(g_direct_system.rx_cnt, 99);
      len = kma_cmd_handler(rx_buff, len, tx_buffer, eREQ_SOURCE_DIRECT);
      if(len)
      {
        drv_uart_send(g_direct_uart_num, tx_buffer, len);
        UPDATE_CNT(g_direct_system.tx_cnt, 99);
        g_direct_system.last_send_time = time_timestamp();
        if (get_firmware_update())
        {
          reset_system( "DIRECT update");
        }
      }
      startTime  = osKernelGetTickCount();
    }
    
    // 링크다운까지 남은 시간 계산
    uint32_t now = osKernelGetTickCount();
    uint32_t elapsed = now - startTime;

    if (elapsed >= DIRECT_TIMEOUT_MS)
    {
      g_direct_system.link_status = eDIRECT_LINK_DOWN;
      g_direct_system.linkdown_remain_ms = 0;
      startTime = now;  // 리셋
    }
    else
    {
      if (g_direct_system.link_status != eDIRECT_LINK_DOWN)
      {
        g_direct_system.linkdown_remain_ms = DIRECT_TIMEOUT_MS - elapsed;
      }
    }

  }
}


void directTask_init(void)
{
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  g_direct_system.link_status = eDIRECT_LINK_IDLE;
  g_direct_system.tx_cnt = 0;
  g_direct_system.rx_cnt = 0;

  uart_config.baud = config_index_to_uart_baud(get_config_app()->direct_baud_index);
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;

  g_direct_uart_num = DRV_UART_8_CDMA;

  if(drv_uart_init(g_direct_uart_num, &uart_config,"Direct") !=1)
  {
    debug_printf("direct_task err\r\n");
  }

  osThreadNew(directTask, NULL, &directTask_attributes);
}