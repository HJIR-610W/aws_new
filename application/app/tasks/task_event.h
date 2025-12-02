#ifndef TASK_ISR_EVENT_H
#define TASK_ISR_EVENT_H

#include <stdint.h>

typedef enum isr_event_cmd_e
{
  eRTC_INT,
  eRAIN_REED_INT,
  eRAIN_HALL_INT,
  eUSER_BTN_INT,
  eUSER_UART_QUAD_1_RX_FULL,
  eUSER_UART_QUAD_2_RX_FULL,
  eUSER_UART_QUAD_3_RX_FULL,
  eUSER_UART_QUAD_4_RX_FULL,
  eUSER_UART_QUAD_5_RX_FULL,
  eUSER_UART_QUAD_6_RX_FULL,
  eUSER_UART_QUAD_7_RX_FULL,
  eUSER_UART_QUAD_8_RX_FULL,
  eUSER_START_CONSOLE,
  eUSER_STOP_CONSOLE,
  eSYSTEM_RESET,
  eUNSUED_CMD = 255
} eISR_EVENT_CMD_t;


typedef struct isr_event_cmd_s
{
  eISR_EVENT_CMD_t cmd;// 이벤트 코드
  char message[7];//추가로 전달할 메시지
}isr_event_cmd_t;



int32_t os_send_event(isr_event_cmd_t *cmd, uint32_t timeOutms);

void eventTask_init(void);
#endif