
#include "task_event.h"

#include "cmsis_os2.h"


#include "bsp.h"
#include "dev_io.h"
#include "FreeRTOS.h"
#include "task_console.h"
#include "Sensors\rain\rain.h"
#include "system_err.h"

const osThreadAttr_t kIsrEventTask_attributes = {
  .name = "isr_event",
  .stack_size = TASK_STACK(TASK_ISR_EVENT_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_ISR_EVENT_DEF),
};

osMessageQueueId_t g_event_msg_q_id;


int32_t os_send_event(isr_event_cmd_t *cmd,uint32_t timeOutms)
{
  int32_t status;

  status = osMessageQueuePut(g_event_msg_q_id, cmd, 0, timeOutms);
  
  return !(osOK==status);
}

void isrEventTask(void *arg)
{
  isr_event_cmd_t event;

  DEBUG_PRINTF("isr task start\r\n");

  while(1)
  {
    if (osMessageQueueGet(g_event_msg_q_id, &event, NULL, osWaitForever) == osOK)
    {
      switch(event.cmd)
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
          task_printf("eUSER_UART_RX_FULL %d\r\n",event.cmd);
          break;
        case eSYSTEM_RESET:
          reset_system("eSYSTEM_RESET");
          break;
         case eUSER_START_CONSOLE:
            start_console((void *)0);
        break;
          case eUSER_STOP_CONSOLE:
          stop_console();
        break;
            default : 
            break;
        }
    }
  }
}


/**
 * @note 시스템에서 우선순위 높게 실시간으로 처리해야하는 업무 담당
 * 다른 task에서는 이벤트 발생시 메시지 형식으로 전달
 * 우량 측정
 * 에러 출력(에러 출력 자체를 해당 task에서는 하지 않는다.)
  */
void eventTask_init(void)
{
  g_event_msg_q_id = osMessageQueueNew(10, sizeof(isr_event_cmd_t), NULL);

  osThreadNew(isrEventTask, NULL, &kIsrEventTask_attributes);
}