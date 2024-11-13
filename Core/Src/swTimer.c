












#include "driver_interface.h"

#include "driver_led.h"
#include "cmsis_os2.h"
#include "main.h"




osTimerId_t myTimerHandle;

void TimerCallback(void *argument) 
{
  driver_t *runLed = (driver_t *)argument;

  driver_led_toggle((driver_t *)runLed);

   
}



void swTimer_init(void)
{

  
#if 0
   myTimerHandle = osTimerNew(TimerCallback, osTimerPeriodic, runLed, NULL);
    
    if (myTimerHandle != NULL) 
    {
        osTimerStart(myTimerHandle, 200); 
    }
    else
    {
        // 타이머 생성 실패 시 오류 처리
        Error_Handler();
    }
    
#endif


}