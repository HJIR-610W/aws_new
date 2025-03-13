


#ifndef DRIVER_485_H
#define DRIVER_485_H

#include "driver_interface.h"
#include "driver_uart_def.h"

#define RS485_A 0
#define RS485_B 1

driver_t *driver_rs485_open(uint32_t num,void *opt);
void driver_rs485_close(driver_t *drv);
int32_t driver_rs485_send(driver_t *drv,uint8_t *pData,uint16_t dataLen);
int32_t driver_rs485_recv(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms);
int32_t driver_rs485_recv_opt(driver_t *drv,uint8_t *pBuff, uint16_t rLen,eUART_RECV_OPT_t cmd,
                              void *opt); 

void driver_rs485_set(driver_t *drv,uart_set_option_t cmd,void *option);

#endif