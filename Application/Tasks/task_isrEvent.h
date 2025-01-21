#ifndef TASK_ISR_EVENT_H
#define TASK_ISR_EVENT_H


typedef enum isrEvent_cmd_e
{
  eRTC_INT,
  eRAIN_REED_INT,
  eRAIN_HALL_INT,
  eUSER_BTN_INT,
  eUNSUED_CMD=255
}eISR_EVENT_CMD_t;


int32_t os_send_isrEvent(eISR_EVENT_CMD_t cmd,uint32_t timeOutms);

void isrEventTask_init(void);
#endif