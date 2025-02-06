
#ifndef TASK_CELLULAR_H
#define TASK_CELLULAR_H

#include <stdint.h>
#include "driver_uart.h"

#include "modem_if.h"
#include "at_cmd.h"
#define SOURCE_TCP   0
#define SOURCE_ASYNC 1


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


void cellularTask_init(void);

#endif