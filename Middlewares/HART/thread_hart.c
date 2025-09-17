
#include "cmsis_os2.h"

const osThreadAttr_t kHartThread_attributes = {
    .name = "HART",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityNormal,
};

//
void hart_thread(void *arg)
{
  while (1)
  {
    osDelay(1000);
  }
}

void hart_init(void)
{
  osThreadNew(hart_thread, NULL, &kHartThread_attributes);
}
