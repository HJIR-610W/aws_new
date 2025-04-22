
#ifndef TASK_TCP_SERVER_H
#define TASK_TCP_SERVER_H

#include "cmsis_os.h"

typedef struct tcp_status_s
{
  eLINK_STATUS_t link_status;
  uint8_t rx_cnt;
  uint8_t tx_cnt;
} tcp_status_t;



void noti_tcpServerTask(uint32_t flag);
void tcpServerTask_init(uint32_t flag);
tcp_status_t *get_tcp_system(void);

#endif