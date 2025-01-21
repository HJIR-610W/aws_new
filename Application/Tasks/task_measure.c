

#include "app_adc.h"
#include "app_rtc.h"

#include "cmsis_os.h"

#include "task_measure.h"

const osThreadAttr_t measureTask_attributes = {
  .name = "measureTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityHigh,
};




void measureTask(void *arg)
{
  
  adc_init();



  while(1)
  {
    osDelay(500);
    rtc_update();
    
  }
  
}

void measureTask_init(void)
{
  osThreadNew(measureTask, NULL, &measureTask_attributes);
}