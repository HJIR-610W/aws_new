

#ifndef DRIVER_RTC_H
#define DRIVER_RTC_H

#include "driver_rtc_define.h"

#define RTC_DS1306       0
#define RTC_RV8803       1
#define RTC_MCU          2


driver_t * driver_rtc_open(int num,void *opt);
int32_t driver_rtc_read(driver_t *driver, DATE_TIME_BUF *t);
void driver_rtc_set(driver_t *driver,rtc_set_option_t cmd,void *opt);

#endif
