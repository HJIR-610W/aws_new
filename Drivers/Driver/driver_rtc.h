

#ifndef DRIVER_RTC_H
#define DRIVER_RTC_H


#include "cmsis_os.h"

#include "driver_interface.h"
#include "time_define.h"
#define RTC_DS1306       0



driver_t * driver_rtc_open(int num);
void driver_rtc_read(driver_t* driver, DATE_TIME_BUF *t);
void driver_rtc_write(driver_t* driver, DATE_TIME_BUF *t);

#endif