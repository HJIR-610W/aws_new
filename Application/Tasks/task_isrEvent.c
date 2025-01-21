
#include "app_rtc.h"
#include "cmsis_os.h"
#include "task_isrEvent.h"
#include "io.h"






const osThreadAttr_t isrEventTask_attributes = {
  .name = "isrEventTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityHigh,
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
        rtc_update();
        break;
        case eRAIN_REED_INT:

        debug_printf("eRAIN_REED_INT\r\n");
        break;
        case eRAIN_HALL_INT:
        debug_printf("eRAIN_HALL_INT\r\n");
        break;
        case eUSER_BTN_INT:
        debug_printf("eUSER_BTN_INT\r\n");
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


  osThreadNew(isrEventTask, NULL, &isrEventTask_attributes);
}