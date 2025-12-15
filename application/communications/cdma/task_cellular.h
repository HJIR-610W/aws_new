
#ifndef TASK_CELLULAR_H
#define TASK_CELLULAR_H

#include <stdint.h>

typedef enum cdma_link_status_e
{
  eCDMA_LINK_IDLE = 0,
  eCDMA_LINK_UP = 1,
  eCDMA_LINK_DOWN = 2
} eCDMA_LINK_STATUS_t;

typedef struct modem_status_s
{
  char num[20];
  int rssi;
  char tx_cnt;
  char rx_cnt;
  eCDMA_LINK_STATUS_t link_status;
  char network_service_msg[50];//네트워크 상태
  char network_name[20];//STK,KT
  uint32_t last_send_time;
  uint32_t last_recv_time;
}cdma_system_t;




cdma_system_t* get_cdma_system(void);

void cellularTask_init(void);



#endif