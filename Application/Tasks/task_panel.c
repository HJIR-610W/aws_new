
#include "cmsis_os2.h"


#include "Panel\panel.h"

const osThreadAttr_t kPanelTask_attributes = {
    .name = "panelTask",
    .stack_size = 768,
    .priority = (osPriority_t)osPriorityBelowNormal,
};



void panelTask(void *arg)
{
  while (1)
  {
    send_panel();
    osDelay(1000);
  }
}

void panelTask_init(void)
{
  panel_init();

  osThreadNew(panelTask, NULL, &kPanelTask_attributes);
}