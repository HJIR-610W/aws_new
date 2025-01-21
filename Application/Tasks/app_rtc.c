
#include "driver_rtc.h"
#include "task_isrEvent.h"
#include "utile_time.h"

driver_t *g_rtc;

void rtcIrqCallBack(void *arg)
{
  os_send_isrEvent(eRTC_INT,0);
}



void rtc_init(void)
{
 rtc_set_irq_cfg_t rtc_cfg;

  g_rtc = driver_rtc_open(RTC_DS1306);

  driver_rtc_read(g_rtc,&Date_Time);

  rtc_cfg.cfg.call    = rtcIrqCallBack;
  rtc_cfg.cfg.name    = "rtc_irq";
  rtc_cfg.cfg.trigger = eDI_FALLING;
  rtc_cfg.cfg.prio    = 5;

  driver_rtc_set(g_rtc,eRTC_SET_IRQ,&rtc_cfg);

}


void rtc_update(void)
{
  DATE_TIME_BUF nt;

  driver_rtc_read(g_rtc,&nt);
  Date_Time = nt;
}

