

#ifndef DRIVER_UART_DEF_H
#define DRIVER_UART_DEF_H

#include <stdint.h>


#define PARITY_NONE  0
#define PARITY_ODD   1
#define PARITY_EVEN  2

#define UART_DATA_LEN_8 0
#define UART_DATA_LEN_9 1

#define UART_STOP_BIT_1 1

typedef struct uart_config_s
{
  int baud;
  uint8_t parity_index;
  uint8_t stop_bit;
  uint8_t dataLen;
} uart_config_t;

typedef enum
{
  eUART_SET_CONFIG,
} eUART_SET_OPTION_t;

typedef enum
{
  eUART_GET_CONFIG  // 설정값 읽기
} eUART_GET_OPTION_t;

#endif
