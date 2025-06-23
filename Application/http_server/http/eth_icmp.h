

#ifndef ETH_ICMP_H
#define ETH_ICMP_H

#include <stdint.h>

typedef struct 
{
  char dest_ip[16];// 목적지 주소 111.111.111.111\0 
  int32_t cnt;         // ping 시도 횟수
}ping_req_t;

void create_pingTask(ping_req_t *pPingReq);

#endif