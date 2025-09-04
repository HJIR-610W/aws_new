#include "cmsis_os2.h"


#include "bsp.h"
#include "dev_io.h"
#include "task_isrEvent.h"
#include "FreeRTOS.h"
#include "Sensors\rain\rain.h"
#include "system_err.h"

const osThreadAttr_t kIsrEventTask_attributes = {
  .name = "isr_event",
  .stack_size = TASK_STACK(TASK_ISR_EVENT_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_ISR_EVENT_DEF),
};

osMessageQueueId_t g_event_msg_q_id;
eISR_EVENT_CMD_t g_isrEventCmd;

int32_t os_send_event(eISR_EVENT_CMD_t cmd,uint32_t timeOutms)
{
  int32_t status;

  status = osMessageQueuePut(g_event_msg_q_id, &cmd, 0, timeOutms);
  
  return !(osOK==status);
}

void isrEventTask(void *arg)
{
  eISR_EVENT_CMD_t cmd;

  DEBUG_PRINTF("isr task start\r\n");

  while(1)
  {
    if (osMessageQueueGet(g_event_msg_q_id, &cmd, NULL, osWaitForever) == osOK)
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
        case eUSER_UART_QUAD_1_RX_FULL:
        case eUSER_UART_QUAD_2_RX_FULL:
        case eUSER_UART_QUAD_3_RX_FULL:
        case eUSER_UART_QUAD_4_RX_FULL:
        case eUSER_UART_QUAD_5_RX_FULL:
        case eUSER_UART_QUAD_6_RX_FULL:
        case eUSER_UART_QUAD_7_RX_FULL:
        case eUSER_UART_QUAD_8_RX_FULL:
          task_printf("eUSER_UART_RX_FULL %d\r\n",cmd);
          break;
        
        default:
          break;
        }
    }
  }
}



void isrEventTask_init(void)
{
  g_event_msg_q_id = osMessageQueueNew(10, sizeof(eISR_EVENT_CMD_t), NULL);


  osThreadNew(isrEventTask, NULL, &kIsrEventTask_attributes);
}