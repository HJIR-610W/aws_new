
#include "bsp_rtc.h"
#include "bsp_stm32_rtc.h"
#include "ds1306.h"
#include "rv8803.h"

#define BSP_RTC_DS1306 0
#define BSP_RTC_RV8803 1
#define BSP_RTC_MCU    2

static driver_t *rtc_driver = NULL;

void bsp_rtc_init(void)
{
  int rtc_number = BSP_RTC_RV8803;

  switch (rtc_number)
  {
    case BSP_RTC_DS1306:
      rtc_driver = ds1306_open();
      break;
    case BSP_RTC_RV8803:
      rtc_driver = rv8803_open();
      break;
    case BSP_RTC_MCU:
      rtc_driver = driver_stm32_rtc_open(STM32_RTC, 0);
      break;
  }
}
int32_t bsp_rtc_read(DATE_TIME_BUF *t)
{
  const rtc_api_t *api;

  if (rtc_driver == NULL)
  {
    return 1;
  }

  api = ((driver_t *)rtc_driver)->api;

  if (api == NULL)
  {
    return 2;
  }

  return api->read(rtc_driver, t);
}

int32_t bsp_rtc_set_date(int year, int month, int day)
{
  return 0;

}
int32_t bsp_rtc_set_time(int year, int month, int day)
{
return 0;

}
int32_t bsp_rtc_set(DATE_TIME_BUF *nt)
{
  const rtc_api_t *api;

  if (rtc_driver == NULL)
  {
    return 1;
  }

  if (is_valid_datetime(nt)==false)
  {
    return 1;
  }

    api = ((driver_t *)rtc_driver)->api;

  if (api == NULL)
  {
    return 2;
  }

  api->set(rtc_driver, eRTC_SET_TIME, nt);
  
  return 0;
}