

#ifndef DRIVER_STM32_DI_H
#define DRIVER_STM32_DI_H

#include <stdint.h>

#include "cmsis_os.h"
#include "driver_interface.h"
#include "stm32f4xx_hal.h"

#define STM32_DI_ADC_RDY    0
#define STM32_DI_RTC_IRQ    1

#define DI_RAIN_HALL     2
#define DI_RAIN_REED     3
#define DI_RAIN_HALL_ERR 4


#define STM32_DI_MAX 5
driver_t *stm32_di_open(int num);
int32_t stm32_di_read(driver_t *driver);


#endif
