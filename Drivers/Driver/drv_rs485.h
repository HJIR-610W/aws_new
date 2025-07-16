

#ifndef DRIVER_485_H
#define DRIVER_485_H

#include "driver_uart_def.h"

#define RS485_A 0
#define RS485_B 1
#define RS485_RS232_C 2 // 이 포트는 하드웨어 점퍼를 RS485로 해야 한다. 
#define RS485_RS232_D 3 // 이 포트는 하드웨어 점퍼를 RS485로 해야 한다.


int32_t drv_rs485_init(int32_t num,void *opt);
int32_t drv_rs485_send(int num, uint8_t *pData, uint16_t dataLen);
int32_t drv_rs485_recv(int num, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutms);
int32_t drv_rs485_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms, uint32_t timeout2_ms);
void drv_rs485_set(int num, uart_set_option_t cmd, void *option);
void drv_rs485_get(int num, uart_get_option_t cmd, void *value);
void drv_rs485_flush_rx(int num);

#endif