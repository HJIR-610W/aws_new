

#ifndef DRIVER_485_DEF_H

#define DRIVER_485_DEF_H

#include <stdio.h>


typedef struct rs485_init_s
{
  uint8_t port_num;
  uint32_t baud;
  uint8_t stop;
  uint8_t parityIdx;
} rs485_init_t;

#endif