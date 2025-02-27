




#ifndef DRIVER_STM32_DI_H
#define DRIVER_STM32_DI_H

#include <stdint.h>

#include "driver_uart_def.h"
#include "driver_interface.h"

#define STM32_CDC        0


driver_t *stm32_cdc_open(int num,void *opt);


#endif
