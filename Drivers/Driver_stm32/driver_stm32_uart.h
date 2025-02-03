

#ifndef DRIVER_STM32_UART_H
#define DRIVER_STM32_UART_H

#include "driver_uart_def.h"
#include "driver_interface.h"

//항상 0부터 시작
#define STM32_UART_0_DEBUG 0
#define STM32_UART_1_CDMA  1 
#define STM32_UART_2_SDI   2

#define STM32_UART_MAX 3

driver_t *stm32_uart_open(int num,void *opt);




#endif
