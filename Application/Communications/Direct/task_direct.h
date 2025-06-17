
#ifndef TASK_DIRECT_H
#define TASK_DIRECT_H

#include <stdint.h>

typedef enum direct_link_status_e
{
  eDIRECT_LINK_IDLE=0,
  eDIRECT_LINK_UP=1,
  eDIRECT_LINK_DOWN=2
}eDIRECT_LINK_STATUS_t;


typedef struct direct_status_s
{
  eDIRECT_LINK_STATUS_t link_status;
  uint8_t rx_cnt;
  uint8_t tx_cnt;
  uint32_t last_send_time;
  uint32_t last_recv_time;
  uint32_t linkdown_remain_ms;
} direct_system_t;

direct_system_t *get_direct_system(void);

void directTask_init(void);

#endif