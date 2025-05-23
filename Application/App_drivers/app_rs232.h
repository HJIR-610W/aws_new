
#ifndef APP_RS232_H
#define APP_RS232_H

#include <stdint.h>
#include <stdbool.h>
#include "driver_uart_def.h"

typedef enum rs232_port_e
{
  eRS232_RS485_A,
  eRS232_RS485_B,
  eRS232_C,
  eRS232_MAX
}eRS232_PORT_t;

uint16_t rs232_get_portList(const char **list,uint16_t listMax);
int32_t uart_num_to_driver_num(int32_t app_uart_num);
#endif