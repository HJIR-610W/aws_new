

#ifndef DRIVER_STM32_UART_H
#define DRIVER_STM32_UART_H

#include "driver_interface.h"
#include "driver_uart_def.h"


#define STM32_UART_0_CDMA 0
#define STM32_UART_1_SDI  1

#define STM32_UART_MAX 2

driver_t *stm32_uart_open(int num, void *opt);

#endif
