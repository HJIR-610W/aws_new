
#include "cmsis_os2.h"
#include "FreeRTOS.h"

#include "Panel\panel.h"

#include "config_app.h"
const osThreadAttr_t kPanelTask_attributes = {
    .name = "panelTask",
    .stack_size = TASK_STACK(TASK_PANEL_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_PANEL_DEF),
};



void panelTask(void *arg)
{
  while (1)
  {
    send_panel();
    osDelay(2000);
  }
}

void panelTask_init(void)
{


  panel_init();

  osThreadNew(panelTask, NULL, &kPanelTask_attributes);
}