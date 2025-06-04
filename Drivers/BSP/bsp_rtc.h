

#ifndef APP_RTC_H
#define APP_RTC_H
#include "util_time.h"
#include "driver_rtc.h"
void bsp_rtc_init(void);
void bsp_rtc_update(void);
void bsp_rtc_set(DATE_TIME_BUF *ct);
#endif