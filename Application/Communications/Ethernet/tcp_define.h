#ifndef __TCP_DEFINE_H__
#define __TCP_DEFINE_H__

#include <stdint.h>


typedef enum link_status_e
{
  eLINK_IDLE=0,
  eLINK_UP=1,
  eLINK_DOWN=2
}eLINK_STATUS_t;

typedef struct tcp_status_s
{
  eLINK_STATUS_t link_status;
  uint8_t rx_cnt;
  uint8_t tx_cnt;
  char *client_ip_str;
  uint32_t last_recv_time;
  uint32_t last_send_time;
} tcp_system_t;

#endif
