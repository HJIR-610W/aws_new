

#ifndef DRIVER_STM32_DI_H
#define DRIVER_STM32_DI_H

#include <stdint.h>

#include "driver_interface.h"
#include "driver_di_def.h"

#define STM32_DI_0_ADC_RDY       0
#define STM32_DI_1_RTC_IRQ       1
#define STM32_DI_RAIN_REED     2
#define STM32_DI_RAIN_HALL     3
#define STM32_DI_RAIN_HALL_ERR 4

#define STM32_DI_QUAD_UARTA_1  5
#define STM32_DI_QUAD_UARTB_2  6
#define STM32_DI_QUAD_UARTC_3  7
#define STM32_DI_QUAD_UARTD_4  8
#define STM32_DI_QUAD_UARTA_5  9
#define STM32_DI_QUAD_UARTB_6 10
#define STM32_DI_QUAD_UARTC_7 11
#define STM32_DI_QUAD_UARTD_8 12

#define STM32_DI_HART_CD       13

#define STM32_DI_MAX          14



driver_t *stm32_di_open(int num,void *opt);
int32_t stm32_di_read(driver_t *driver);
void stm32_di_set(driver_t *drv,di_set_option_t cmd,void *option);

#endif
