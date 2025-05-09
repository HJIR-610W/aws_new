#include <stdio.h>
#include <string.h>
#include "cli_key_code.h"
#include "dev_io.h"
#include "driver_485.h"



#define RS485_PORT_MAX 2
void test_rs485(void)
{
  uart_config_t uart_config;
  int len;
  driver_t *port[RS485_PORT_MAX];
  char buff[30];
  char rx_buff[10];

  uart_config.baud = 57600;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  port[0] = driver_rs485_open(RS485_A, &uart_config);
  port[1] = driver_rs485_open(RS485_B, &uart_config);

  while(1)
  {
    for (int i = 0; i < RS485_PORT_MAX; i++)
    {
      snprintf(buff,sizeof(buff),"port %d\r\n",i);
      len = driver_rs485_send(port[i], buff, strlen(buff));
      if(len<0)
      {
        snprintf(buff, sizeof(buff), "port %d failed\r\n", i);
        debug_printf(buff);
      }
      len = driver_rs485_recv(port[i], rx_buff, sizeof(rx_buff), 1000);
      if(len)
      {
        driver_rs485_send(port[i], rx_buff, len);
      }
      if (get_key(100) == KEY_CODE_CTRL_Q)
      {
        return;
      }
    }

  }
}