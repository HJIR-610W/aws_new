

#include "cmsis_os.h"
#include "driver_rtc.h"
#include "io.h"
#include "utile_time.h"


const osThreadAttr_t loggingTask_attributes = {
  .name = "loggingTask",
  .stack_size = 2048,//2048바이트가 할당됨 하지만 4바이트 단위로 스택은 구성됨
  .priority = (osPriority_t) osPriorityNormal,
};


typedef enum logging_cmd_e
{
  eLOGGING_SCHEDULE,
  eLOGGING_LOG,
  eLOGGING_SENSOR
}eLOGGING_CMD_t;


void loggingTask(void *arg)
{
  DATE_TIME_BUF nt;
  driver_t *rtc;

  rtc = driver_rtc_open(RTC_DS1306);
  while(1)
  {
    osDelay(1000);
    driver_rtc_read(rtc,&nt);
    debug_printf("%4d-%2d-%02d %02d:%02d:%02d\r\n",nt.Year,nt.Month,nt.Day,
                                        nt.Hour,nt.Min,nt.Sec);

  }
}



void loggingTask_init(void)
{
 
  osThreadNew(loggingTask, NULL, &loggingTask_attributes);

}