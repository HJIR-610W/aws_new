

#ifndef TLC16C554_H
#define TLC16C554_H


#include <stdint.h>
#include "driver_interface.h"
#include "driver_uart_def.h"

//항상 0부터 시작해야함
#define TL16C554_UART_1_D_SUB   0
#define TL16C554_UART_2_TTL_TTL 1
#define TL16C554_UART_3_RS232_A 2
#define TL16C554_UART_4_RS232_B 3
#define TL16C554_UART_5_RS485_A 4
#define TL16C554_UART_6_RS485_B 5
#define TL16C554_UART_7_RS232_C 6
#define TL16C554_UART_8_RS232_D 7

#define TL16C554_UART_MAX 8


driver_t *tls16c554_open(uint32_t num,void *opt);


#endif
