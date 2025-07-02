

#ifndef DRIVER_STM32_DO_H
#define DRIVER_STM32_DO_H

#include <stdint.h>

#include "driver_interface.h"
#include "driver_do_define.h"

#define STM32_DO_POWER_CDMA        0
#define STM32_DO_ADC_NCS         1
#define STM32_DO_FRAM_CS         2
#define STM32_DO_RTC_CS          3
#define STM32_DO_FLASH_CS        4
#define STM32_DO_HART_SEL        5
#define STM32_DO_HART_RTS        6
#define STM32_DO_POWER_24V       7
#define STM32_DO_HART_RESET      8
#define STM32_DO_LCD_RESET        9
#define STM32_DO_DIR_SDI        10
#define STM32_DO_DIR_RS485_A    11
#define STM32_DO_DIR_RS485_B    12
#define STM32_DO_DIR_RS485_C    13
#define STM32_DO_DIR_RS485_D    14
#define STM32_DO_POWER_RAIN_DECT_DIGITAL  15
#define STM32_DO_POWER_RAIN_DECT_ANALOG 16
#define STM32_DO_LCD_CS 17
#define STM32_DO_MAX           18


driver_t *stm32_do_open(int num,void *opt);
void stm32_do_low(driver_t *driver);
void stm32_do_high(driver_t *driver);

#endif
