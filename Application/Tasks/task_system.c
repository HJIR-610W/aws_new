
#include "app_rtc.h"
#include "cmsis_os2.h"
#include "task_isrEvent.h"
#include "dev_io.h"
#include "app_rtc.h"



const osThreadAttr_t systemTask_attributes = {
  .name = "systemTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityHigh,
};





void systemTask(void *arg)
{
  while(1)
  {
    rtc_update();
    osDelay(250);
  }
}





void systemTask_init(void)
{

  osThreadNew(systemTask, NULL, &systemTask_attributes);
}