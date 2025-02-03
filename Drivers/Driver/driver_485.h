


#ifndef DRIVER_485_H
#define DRIVER_485_H

#include "driver_interface.h"


#define RS485_A 0
#define RS485_B 1

#define RS485_CMD_SET_BAUD 0

driver_t *driver_rs485_open(uint32_t num,void *opt);

int32_t driver_rs485_send(driver_t *drv,uint8_t *pData,uint16_t dataLen);
int32_t driver_rs485_recv(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms);
void driver_rs485_set(driver_t *drv,uint8_t cmd,void *option);

#endif