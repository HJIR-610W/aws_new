
#include "cmsis_os2.h"
#include "config_app.h"
#include "dev_io.h"
#include "driver_di.h"
#include "driver_do.h"
#include "driver_uart.h"

#include "cli_key_code.h"
#include "hart_parser.h"

#define HART_TX_ON() driver_do_low(g_hart_rts)
#define HART_TX_OFF() driver_do_high(g_hart_rts)
#define IS_HART_CD() driver_di_read(g_hart_cd)
#define HART_SEND(data, len) driver_uart_send(g_hart_uart, data, len)
#define HART_RECV(buff, buffSize, timeout) driver_uart_recv(g_hart_uart, buff, buffSize, timeout)

#define HART_POWER_ON() driver_do_high(g_power_24)
#define HART_POWER_OFF() driver_do_low(g_power_24)

#define HART_RESET_L()
#define HART_RESET_H()

driver_t *g_hart_uart;
driver_t *g_hart_rts;
driver_t *g_hart_cd;
driver_t *g_hart_sel;
driver_t *g_power_24;
driver_t *g_hart_reset;

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
  do_config_t do_config;

  uart_config.baud = 1200;
  uart_config.parityIdx = PARITY_ODD;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  g_hart_uart = driver_uart_open(UART_5_EXT_D, &uart_config);
  g_hart_cd = driver_di_open(DI_HART_CD, 0);

  do_config.mode = DO_OUT_PP;
  do_config.pullup = DO_NO_PULL;

  g_hart_rts = driver_do_open(DO_HART_RTS, &do_config);
  g_hart_sel = driver_do_open(DO_HART_SEL, &do_config);
  g_power_24 = driver_do_open(DO_POWER_HART_24V, &do_config);

  driver_do_high(g_power_24);  // HART 24V를 공급

  g_hart_reset = driver_do_open(DO_HART_RESET, &do_config);

  driver_do_high(g_hart_sel);
  driver_do_low(g_hart_reset);  // HART 리셋
  osDelay(10);
  driver_do_high(g_hart_reset);

  hart_task(0);
}