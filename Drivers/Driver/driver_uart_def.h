

#ifndef DRIVER_UART_DEF_H
#define DRIVER_UART_DEF_H


typedef struct uart_baud_config_s
{
  int num;
  int baud;
  int parity;
}uart_baud_config_t;


typedef enum uart_set_cmd_s
{
  eUART_SET_CONFIG,
  eUART_SET_TIMEOUT
}eUART_SET_CMD_t;

#endif
