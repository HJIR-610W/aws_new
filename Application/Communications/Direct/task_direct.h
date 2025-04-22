
#ifndef TASK_DIRECT_H
#define TASK_DIRECT_H

#include <stdint.h>

typedef enum direct_link_status_e
{
  eDIRECT_LINK_IDLE=-1,
  eDIRECT_LINK_UP=0,
  eDIRECT_LINK_DOWN=1
}eDIRECT_LINK_STATUS_t;


typedef struct direct_status_s
{
  eDIRECT_LINK_STATUS_t link_status;
  uint8_t rx_cnt;
  uint8_t tx_cnt;
} direct_status_t;

direct_status_t *get_direct_system(void);

void directTask_init(void);

#endif