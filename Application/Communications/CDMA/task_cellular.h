
#ifndef TASK_CELLULAR_H
#define TASK_CELLULAR_H

#include <stdint.h>

#include "driver_uart.h"

#include "modem_if.h"
#include "at_cmd.h"

#define SOURCE_TCP   0
#define SOURCE_ASYNC 1


typedef enum modem_link_e
{
  eLINK_DISCONNECTED,
  eLINK_CONNECTED
}eLINK_t;

typedef enum modem_model_e
{
  eNTLE9607
}eMODEM_MODEL_t;

typedef struct modem_config_s
{
  uint8_t ip[4];
  uint16_t port;
  eMODEM_MODEL_t model;
  uint16_t connection_timeoutms;
}modem_config_t;

typedef struct modem_status_s
{
  char num[20];
  char rssi;
  char txCnt;
  char rxCnt;
  eLINK_t link_status;
  char network_service_msg[50];//네트워크 상태
  char network_name[20];//STK,KT
  
}modem_status_t;


typedef struct
{
	iCellular_t cellular;
	atCmd_t *atCmd;
	uint16_t atCmdCnt;
}cellular_t;


bool is_modemBoot(void);
bool is_modemServerErr(void);
uint32_t os_recv_ack(uint8_t source,char* rBuff, uint32_t buffSize,uint32_t timeOutMs);
void os_put_tcpData(uint8_t* data, uint16_t dataLen);
void os_send_at(uint8_t source,const char* data, uint32_t dataLen,const char *ack, uint32_t timeOutMs);
uint32_t os_recv_tcp(uint8_t* pBuff, uint16_t buffSize, uint32_t* pLen, uint32_t timeOutMs);


extern driver_t *cdma_driver;

extern modem_status_t g_modem_status;
void cellularTask_init(void);

extern iCellular_t *_iCellular;

#endif