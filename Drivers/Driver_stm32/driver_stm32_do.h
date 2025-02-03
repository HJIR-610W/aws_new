

#ifndef DRIVER_STM32_DO_H
#define DRIVER_STM32_DO_H

#include <stdint.h>

#include "driver_interface.h"
#include "driver_do_define.h"

#define STM32_DO_PWR_CDMA        0
#define STM32_DO_ADC_NCS         1
#define STM32_DO_FRAM_CS         2
#define STM32_DO_RTC_CS          3
#define STM32_DO_FLASH_CS        4
#define STM32_DO_CON_PWR_232_A   5
#define STM32_DO_CON_PWR_232_B   6
#define STM32_DO_CON_PWR_485     7
#define STM32_DO_CON_PWR_TC      8
#define STM32_DO_CON_PWR_DSEN    9
#define STM32_DO_CON_PWR_ASEN   10
#define STM32_DO_CON_PWR_ASEN_A 11
#define STM32_DO_CON_PWR_ASEN_B 12
#define STM32_DO_CON_PWR_ASEN_C 13
#define STM32_DO_CON_PWR_ASEN_D 14
#define STM32_DO_DIR_SDI        15
#define STM32_DO_DIR_RS485_A    16
#define STM32_DO_DIR_RS485_B    17

#define STM32_DO_MAX            18


driver_t *stm32_do_open(int num,void *opt);
void stm32_do_low(driver_t *driver);
void stm32_do_high(driver_t *driver);

#endif
