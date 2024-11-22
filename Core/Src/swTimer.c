












#include "driver_interface.h"
#include "driver_rtc.h"
#include "driver_led.h"
#include "cmsis_os2.h"
#include "main.h"
#include "utile_time.h"



osTimerId_t myTimerHandle;

void TimerCallback(void *argument) 
{
  driver_t *runLed = (driver_t *)argument;

  driver_led_toggle((driver_t *)runLed);

   
}




void readRTCCallBack(void *arg)
{

  DATE_TIME_BUF nt;

  driver_rtc_read((driver_t *)arg,&nt);

  time_set(&nt);
}

void swTimer_init(void)
{

  driver_t *rtc = driver_rtc_open(RTC_DS1306);

   myTimerHandle = osTimerNew(readRTCCallBack, osTimerPeriodic, rtc, NULL);
    
    if (myTimerHandle != NULL) 
    {
        osTimerStart(myTimerHandle, 500); 
    }
    else
    {
        // 타이머 생성 실패 시 오류 처리
        Error_Handler(__FILE__,__LINE__);
    }
    



}