
#include "driver_rtc.h"

#include "task_isrEvent.h"
#include "util_time.h"

driver_t *g_rtc;

void rtcIrqCallBack(void *arg)
{
  os_send_isrEvent(eRTC_INT,0);
}

void bsp_rtc_init(void)
{
  g_rtc = driver_rtc_open(RTC_DS1306,0);

  driver_rtc_read(g_rtc,&Date_Time);

}


void bsp_rtc_update(void)
{
  DATE_TIME_BUF nt={0};
  

  if(driver_rtc_read(g_rtc,&nt) !=0)
  {
    driver_rtc_read(g_rtc,&nt);
  }
  Date_Time = nt;
}


void bsp_rtc_set(DATE_TIME_BUF *ct)
{
  driver_rtc_set(g_rtc,eRTC_SET_TIME,ct);
}