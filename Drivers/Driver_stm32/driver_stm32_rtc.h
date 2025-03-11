
#ifndef DRIVER_STM32_RTC_H
#define DRIVER_STM32_RTC_H


#include "driver_interface.h"
#include "driver_rtc_define.h"

#define STM32_RTC 0
driver_t *driver_stm32_rtc_open(uint32_t num,void *opt);

#endif