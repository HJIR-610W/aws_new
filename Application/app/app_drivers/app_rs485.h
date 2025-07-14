
#ifndef APP_RS485_H
#define APP_RS485_H

#include <stdint.h>
#include <stdbool.h>

typedef enum RS485_PORT_e
{
  eAPP_RS485_A,
  eAPP_RS485_B,
  eAPP_RS485_C,
  eAPP_RS485_D,
  eAPP_RS485_MAX
} eRS485_PORT_t;


uint16_t rs485_get_portList(const char **list,uint16_t listMax);


#endif