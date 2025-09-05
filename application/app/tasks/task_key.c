
#include "cmsis_os2.h"
#include "FreeRTOS.h"

#include "Panel\panel.h"
#include "app_key.h"

const osThreadAttr_t kKeyTask_attributes = {
    .name = "key",
    .stack_size = TASK_STACK(TASK_KEY_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_KEY_DEF),
};

void keyTask(void *arg)
{

        
  while (1)
  {
    scan_key();
  }
}

void keyTask_init(void)
{
        app_key_init();
  osThreadNew(keyTask, NULL, &kKeyTask_attributes);
}