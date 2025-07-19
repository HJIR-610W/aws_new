#ifndef __TASK_CLIENT_H__
#define __TASK_CLIENT_H__

#include <stdint.h>
#include "tcp_define.h"
void tcpClientTask_init(void);
void noti_tcpClientTask(uint32_t flag);
tcp_system_t *get_tcp_client_system(void);
#endif
