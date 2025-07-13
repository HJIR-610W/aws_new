

#ifndef BSP_RTC_H
#define BSP_RTC_H
#include "driver_rtc_define.h"
#include "util_time.h"

void bsp_rtc_init(void);
int32_t bsp_rtc_read(DATE_TIME_BUF *t);
int32_t bsp_rtc_set_date(int year, int month, int day);
int32_t bsp_rtc_set_time(int year, int month, int day);
int32_t bsp_rtc_set(DATE_TIME_BUF *nt);
#endif