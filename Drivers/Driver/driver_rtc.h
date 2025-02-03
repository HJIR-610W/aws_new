

#ifndef DRIVER_RTC_H
#define DRIVER_RTC_H


#include "cmsis_os.h"

#include "driver_di.h"
#include "driver_interface.h"
#include "time_define.h"
#define RTC_DS1306       0

typedef enum driver_rtc_set_s
{
  eRTC_SET_IRQ
}eRTC_SET_t;

typedef struct rtc_set_irq_cfg_s
{
  di_isr_set_cfg_t cfg;
}rtc_set_irq_cfg_t;



driver_t * driver_rtc_open(int num);
void driver_rtc_read(driver_t* driver, DATE_TIME_BUF *t);
void driver_rtc_write(driver_t* driver, DATE_TIME_BUF *t);

void driver_rtc_set(driver_t *driver,uint8_t cmd,void *opt);
#endif
