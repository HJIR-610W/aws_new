

#ifndef DRIVER_485_H
#define DRIVER_485_H

#include "driver_uart_def.h"
#include "bsp_rs485.h"

#define DRV_RS485_A       BSP_RS485_A
#define DRV_RS485_RS232_C BSP_RS485_RS232_C // 이 포트는 하드웨어 점퍼를 RS485로 해야 한다.
#define DRV_RS485_RS232_D BSP_RS485_RS232_D // 이 포트는 하드웨어 점퍼를 RS485로 해야 한다.

int32_t drv_rs485_init(int32_t num,void *opt);
int32_t drv_rs485_send(int num, uint8_t *pData, uint16_t dataLen);
int32_t drv_rs485_recv(int num, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutms);
int32_t drv_rs485_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms, uint32_t timeout2_ms);
void drv_rs485_set(int num, eUART_SET_OPTION_t cmd, void *option);
void drv_rs485_get(int num, eUART_GET_OPTION_t cmd, void *value);
void drv_rs485_flush_rx(int num);

#endif