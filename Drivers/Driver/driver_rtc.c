
#include <stdio.h>

#include "driver_rtc.h"
#include "driver_stm32_rtc.h"
#include "ds1306.h"
#include "rv8803.h"


driver_t * driver_rtc_open(int num,void *opt)
{
  driver_t *driver=NULL;

  switch(num)
  {
    case RTC_DS1306:
      driver = ds1306_open();
    break;
    case RTC_RV8803:
      driver = rv8803_open();
    break;
    case RTC_MCU:
      driver = driver_stm32_rtc_open(STM32_RTC, 0);
      break;
  }

  return driver;
}

/**
 * 
 */
void driver_rtc_read(driver_t* driver, DATE_TIME_BUF *t)
{
  const rtc_api_t* api;

  if(driver == NULL)
  {
    return;
  }

  api = ((driver_t *)driver)->api;

  if(api == NULL)
  {
    return ;
  }
  
  api->read(driver,t);
 
}


void driver_rtc_set(driver_t *driver,rtc_set_option_t cmd,void *opt)
{
   const rtc_api_t* api;

   if(driver)
   {
     api = ((driver_t *)driver)->api;
    api->set(driver,cmd,opt);
   }

}
