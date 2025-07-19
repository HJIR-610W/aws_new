
#include "drv_rtc.h"
#include "bsp_rtc.h"

void drv_rtc_init(void)
{
  bsp_rtc_init();
}

int32_t drv_rtc_read(DATE_TIME_BUF *t)
{
  return bsp_rtc_read(t);
}

int32_t drv_rtc_set_date(int year,int month,int day)
{
  return bsp_rtc_set_date( year,  month,  day);
}

int32_t drv_rtc_set_time(int hour, int min, int sec)
{
  return bsp_rtc_set_time( hour,  min,  sec);
}

int32_t drv_rtc_set(DATE_TIME_BUF *nt)
{
  return bsp_rtc_set(nt);
}