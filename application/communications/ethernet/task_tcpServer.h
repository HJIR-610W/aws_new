
#ifndef TASK_TCP_SERVER_H
#define TASK_TCP_SERVER_H

#include "cmsis_os2.h"

#include "tcp_define.h"

#define ETH_CLIENT_0 0
#define ETH_CLIENT_1 1
#define ETH_CLIENT_2 2
#define ETH_CLIENT_MAX 3




void noti_tcpServerTask(uint32_t flag);
void tcpServerTask_init(uint32_t flag);
tcp_system_t *get_tcp_system(uint32_t number);

#endif