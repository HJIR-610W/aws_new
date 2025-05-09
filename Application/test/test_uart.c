#include <stdio.h>
#include <string.h>
#include "cli_key_code.h"
#include "dev_io.h"
#include "driver_uart.h"

driver_t *g_rs232_cdma;

driver_t *g_rs232_vhf;
driver_t *g_rs232_web;
driver_t *g_rs232_a;
driver_t *g_rs232_b;
driver_t *g_rs232_c;
driver_t *g_rs232_d;

#define UART_PORT_MAX 6
void test_uart(void)
{
  uart_config_t uart_config;
  int len;
  driver_t *port[UART_PORT_MAX];
  char buff[30];
  char rx_buff[10];

  uart_config.baud = 57600;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  port[0] = driver_uart_open(UART_0_D_SUB_0, &uart_config);
  port[1] = driver_uart_open(UART_1_TTL, &uart_config);
  port[2] = driver_uart_open(UART_2_EXT_A, &uart_config);
  port[3] = driver_uart_open(UART_3_EXT_B, &uart_config);
  port[4] = driver_uart_open(UART_4_EXT_C, &uart_config);
  port[5] = driver_uart_open(UART_5_EXT_D, &uart_config);

  while(1)
  {
    for (int i = 0; i < UART_PORT_MAX;i++)
    {
      snprintf(buff,sizeof(buff),"port %d\r\n",i);
      len = driver_uart_send(port[i],buff,strlen(buff));
      if(len<0)
      {
        snprintf(buff, sizeof(buff), "port %d failed\r\n", i);
        debug_printf(buff);
      }
      len = driver_uart_recv(port[i], rx_buff, sizeof(rx_buff), 1000);
      if(len)
      {
        driver_uart_send(port[i],rx_buff,len);
      }
      if (get_key(100) == KEY_CODE_CTRL_Q)
      {
        return;
      }
    }

  }
}