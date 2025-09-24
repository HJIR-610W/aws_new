
#ifndef APP_RS485_H
#define APP_RS485_H

#include <stdint.h>
#include <stdbool.h>

typedef enum RS485_PORT_e
{
  eAPP_RS485_RS232_A,
  eAPP_RS485_RS232_B,
  eAPP_RS485_C,
  eAPP_RS485_MAX
} eRS485_PORT_t;

uint16_t drv_rs485_get_portList(const char **list,uint16_t listMax);
int32_t rs485_num_to_driver_num(int32_t app_rs485_num);

#endif