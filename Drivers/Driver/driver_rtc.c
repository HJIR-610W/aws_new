
#include <stdio.h>

#include "driver_rtc.h"
#include "ds1306.h"



driver_t * driver_rtc_open(int num,void *opt)
{
  driver_t *driver=NULL;

    switch(num)
  {
    case RTC_DS1306:
    driver = ds1306_open();
    break;
    case RTC_MCU:
    break;
  }

  return driver;
}

void driver_rtc_read(driver_t* driver, DATE_TIME_BUF *t)
{
   const rtc_api_t* api = ((driver_t *)driver)->api;

  api->read(driver,t);
}


void driver_rtc_set(driver_t *driver,rtc_set_option_t cmd,void *opt)
{
   const rtc_api_t* api = ((driver_t *)driver)->api;

  api->set(driver,cmd,opt);

}
