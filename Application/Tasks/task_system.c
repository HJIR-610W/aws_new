#include "cmsis_os2.h"

#include "app_rtc.h"

const osThreadAttr_t kSystemTask_attributes = {
  .name = "systemTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityLow,
};


void systemTask(void *arg)
{
  while(1)
  {
    rtc_update();
    osDelay(500);
  }
}


void systemTask_init(void)
{
  osThreadNew(systemTask, NULL, &kSystemTask_attributes);
}