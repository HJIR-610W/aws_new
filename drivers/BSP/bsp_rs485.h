
#ifndef BSP_RS485_H
#define BSP_RS485_H

#define BSP_RS485_A 0
#define BSP_RS485_B 1
#define BSP_RS485_RS232_C 2
#define BSP_RS485_RS232_D 3

#define BSP_RS485_MAX 4
#include <stdint.h>

#include "driver_uart_def.h"
int32_t bsp_rs485_init(int32_t num,void *opt);
int32_t bsp_rs485_send(int num, uint8_t *pData, uint16_t dataLen);
int32_t bsp_rs485_recv(int num, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutms);
int32_t bsp_rs485_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms, uint32_t timeout2_ms);
void bsp_rs485_set(int num, uart_set_option_t cmd, void *option);
void bsp_rs485_get(int num, uart_get_option_t cmd, void *value);
void bsp_rs485_flush_rx(int num);

#endif