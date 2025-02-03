

#ifndef TLC16C554_H
#define TLC16C554_H


#include <stdint.h>
#include "driver_interface.h"
#include "driver_uart_def.h"

//항상 0부터 시작해야함
#define TL16C554_UART_0_D_SUB   0
#define TL16C554_UART_1_TTL_TTL 1
#define TL16C554_UART_EXT3      2
#define TL16C554_UART_EXT4      3
#define TL16C554_UART_4_RS485_A 4
#define TL16C554_UART_5_RS485_B 5
#define TL16C554_UART_6_EXT1    6
#define TL16C554_UART_7_EXT2    7

#define TL16C554_UART_MAX 8


driver_t *tls16c554_open(uint32_t num,void *opt);


#endif
