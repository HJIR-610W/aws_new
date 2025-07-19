#include "cmsis_os2.h"


#include "bsp.h"
#include "dev_io.h"
#include "task_isrEvent.h"
#include "FreeRTOS.h"
#include "Sensors\rain\rain.h"

const osThreadAttr_t kIsrEventTask_attributes = {
  .name = "isr_event",
  .stack_size = TASK_ISR_EVENT_STACK_SIZE,
  .priority = (osPriority_t) osPriorityRealtime2,
};

osMessageQueueId_t g_isrEventMessageQueue;
eISR_EVENT_CMD_t g_isrEventCmd;

int32_t os_send_isrEvent(eISR_EVENT_CMD_t cmd,uint32_t timeOutms)
{
  int32_t status;
  status = osMessageQueuePut(g_isrEventMessageQueue, &cmd, 0, timeOutms);
  
  return !(osOK==status);
}

void isrEventTask(void *arg)
{
  eISR_EVENT_CMD_t cmd;

  while(1)
  {
    if (osMessageQueueGet(g_isrEventMessageQueue, &cmd, NULL, osWaitForever) == osOK)
    {
      switch(cmd)
      {
        case eRTC_INT:

        break;
        case eRAIN_REED_INT:
        task_printf("eRAIN_REED_INT\r\n");
        increase_rain();
        break;
        case eRAIN_HALL_INT:
        task_printf("eRAIN_HALL_INT\r\n");
          increase_rain();
          break;
        case eUSER_BTN_INT:
        task_printf("eUSER_BTN_INT\r\n");
          break;
        default:
        break;
      }
    }
  }
}



void isrEventTask_init(void)
{
  g_isrEventMessageQueue = osMessageQueueNew(10, sizeof(eISR_EVENT_CMD_t), NULL);


  osThreadNew(isrEventTask, NULL, &kIsrEventTask_attributes);
}