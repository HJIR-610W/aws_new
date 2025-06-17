#ifndef AWS_MONITOR_H
#define AWS_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "task_cellular.h"
#include "task_system.h"
#include "config_app.h"
#include "task_tcpServer.h"
#include "task_direct.h"

//서버에서 모니터링하기위해 전용 구조체를 정의
// 개별 구조체를 하나로 합친다.
typedef struct aws_monitor_s
{
  system_t system;
  cdma_system_t cdma_system;
  direct_system_t direct_system;
  tcp_system_t eth_system[ETH_CLIENT_MAX];

} aws_monitor_t;

void make_aws_monitor_frame(aws_monitor_t *p_monitor);
#ifdef __cplusplus
}
#endif

#endif // AWS_MONITOR_H