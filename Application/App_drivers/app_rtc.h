

#ifndef APP_RTC_H
#define APP_RTC_H
#include "utile_time.h"
#include "driver_rtc.h"
void rtc_init(void);
void rtc_update(void);
void rtc_set(DATE_TIME_BUF *ct);
#endif