
#include "cmsis_os2.h"
#include "config_app.h"
#include "dev_io.h"
#include "drv_di.h"
#include "drv_do.h"
#include "drv_rs232.h"

#include "cli_key_code.h"
#include "hart_parser.h"
#include "drv_power.h"
#include "bsp_do.h"
#include "bsp_di.h"
#define HART_TX_ON() bsp_do_low(BSP_DO_HART_RTS)
#define HART_TX_OFF() bsp_do_high(BSP_DO_HART_RTS)
#define IS_HART_CD() bsp_di_read(BSP_DI_HART_CD)
#define HART_SEND(data, len) drv_uart_send(g_hart_uart_num, data, len)
#define HART_RECV(buff, buffSize, timeout) drv_uart_recv(g_hart_uart_num, buff, buffSize, timeout)

#define HART_POWER_ON() drv_power_on(DRV_POWER_HART_24V)
#define HART_POWER_OFF() drv_power_off(DRV_POWER_HART_24V)

#define HART_RESET_L()
#define HART_RESET_H()

int32_t g_hart_uart_num;

const osThreadAttr_t hardTask_attributes = {
    .name = "hartTask",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityNormal1,
};

int32_t hart_send(uint8_t *cmd, uint16_t dataLen)
{
  int32_t len=0;
  HART_TX_ON();
  osDelay(2);
  len = HART_SEND(cmd, dataLen);
  osDelay(2);
  HART_TX_OFF();

  return len;
}


void hart_task(void *arg)
{
  uint8_t buff[50];
  uint8_t cmd[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x02, 0x80, 0x00, 0x00, 0x82};
  //유니버설 명령어 0,HART Ver
  int32_t len;

  while (1)
  {
    len  = hart_send(cmd, sizeof(cmd));
    if(len < 0)
    {
      io_printf("send failed\r\n");
    }
    len = HART_RECV(buff, sizeof(buff), 1000);
    if (len)
    {
      hart_parse(buff,len);
    }
    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      break;
    }
  }
}

void test_hart(void)
{
  uart_config_t uart_config = {.dataLen = UART_DATA_LEN_8, .stop_bit = 0};


  uart_config.baud = 1200;
  uart_config.parityIdx = PARITY_ODD;
  uart_config.stop_bit = UART_STOP_BIT_1;
  uart_config.dataLen = UART_DATA_LEN_8;

  g_hart_uart_num = DRV_UART_5_EXT_D;

  drv_rs232_init(g_hart_uart_num, &uart_config);

  drv_power_on(DRV_POWER_HART_24V); 
  
  bsp_do_high(BSP_DO_HART_SEL);
  bsp_do_low(BSP_DO_HART_RESET);  
  osDelay(10);
  bsp_do_high(BSP_DO_HART_RESET);

  hart_task(0);
}