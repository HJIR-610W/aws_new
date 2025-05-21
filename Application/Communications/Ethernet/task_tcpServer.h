
#ifndef TASK_TCP_SERVER_H
#define TASK_TCP_SERVER_H

#include "cmsis_os.h"

#include "tcp_define.h"



void noti_tcpServerTask(uint32_t flag);
void tcpServerTask_init(uint32_t flag);
tcp_status_t *get_tcp_system(void);

#endif