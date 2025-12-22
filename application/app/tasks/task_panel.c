
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "Panel\panel.h"
#include "user_heap.h"
#include "config_app.h"
#include "debug_io.h"

const osThreadAttr_t kPanelTask_attributes = {
    .name = "panel",
    .stack_size = TASK_STACK(TASK_PANEL_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_PANEL_DEF),
};
static StaticTask_t s_panelTcb;
static osThreadId_t g_panelTaskId = NULL;

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

#if 0 //freertos heap사용
  osThreadNew(panelTask, NULL, &kPanelTask_attributes);
#endif

  #if 1
const uint32_t stack_size = TASK_STACK(TASK_PANEL_DEF);
    void* stack_mem = user_malloc(stack_size);
    if (stack_mem != NULL) {
        osThreadAttr_t custom_attr = kPanelTask_attributes;
        custom_attr.stack_mem = stack_mem;
        custom_attr.stack_size = stack_size;
        custom_attr.cb_mem  = &s_panelTcb;
        custom_attr.cb_size = sizeof(s_panelTcb);

        g_panelTaskId = osThreadNew(panelTask, NULL, &custom_attr);
        if (g_panelTaskId == NULL) {
            debug_printf("Panel Task: Task creation failed\r\n");
            user_free(stack_mem);
        }
    } else {
        debug_printf("Panel Task: Failed to allocate memory for task stack\r\n");
    }
#endif
}