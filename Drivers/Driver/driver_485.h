


#ifndef DRIVER_485_H
#define DRIVER_485_H

#include "driver_interface.h"


#define RS485_A 0
#define RS485_B 1

driver_t *driver_rs485_open(uint32_t num);
void driver_rs485_sends(driver_t *drv,uint8_t *pData,uint16_t dataLen);


uint16_t driver_rs485_recv(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms);

#endif