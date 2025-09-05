
#ifndef DRV_RTC_H
#define DRV_RTC_H

#include "util_time.h"
#include "drv_rtc_define.h"


void drv_rtc_init(void);
int32_t drv_rtc_read(DATE_TIME_BUF *t);
int32_t drv_rtc_set_date(int year, int month, int day);
int32_t drv_rtc_set_time(int year, int month, int day);
int32_t drv_rtc_set(DATE_TIME_BUF *nt);

#endif