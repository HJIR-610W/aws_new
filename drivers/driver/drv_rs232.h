

#ifndef DRV_RS232_H
#define DRV_RS232_H

#include "driver_uart_def.h"
#include "bsp_uart.h"

#define DRV_UART_2_EXT_A BSP_UART_2_EXT_A // 사용자0
#define DRV_UART_3_EXT_B BSP_UART_3_EXT_B // 사용자1
#define DRV_UART_4_EXT_C BSP_UART_4_EXT_C // 사용자2
#define DRV_UART_5_EXT_D BSP_UART_5_EXT_D // 사용자3
#define DRV_UART_8_CDMA  BSP_UART_8_CDMA  // CDMA
#define DRV_UART_10_CDC  BSP_UART_10_CDC  // USB 디버깅


int32_t drv_uart_init(int32_t num, void *opt);
void drv_uart_close(int num);
int32_t drv_uart_send(int num, const uint8_t *pData, uint16_t dataLen);
int32_t drv_uart_recv(int num, uint8_t *pBuff, uint16_t rLen,
                      uint32_t timeOutMs);
int32_t drv_uart_recv_crlf(int num, char *pBuff, uint16_t bSize,
                           uint32_t tout_ms);
void drv_uart_set(int num, eUART_SET_OPTION_t cmd, void *para);

// 밑에 두함수는 대체 필요
int32_t drv_uart_get_char(int num, uint8_t *pBuff, uint16_t rLen);
int32_t drv_uart_get_charNonBlocking(int num, uint8_t *pBuff);

void drv_uart_flush_rx(int num);

int32_t drv_uart_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size,
                          uint32_t timeout1_ms, uint32_t timeout2_ms);
void drv_uart_get(int num, eUART_GET_OPTION_t cmd, void *para);
int32_t drv_uart_recv_ll(int num, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutMs);

int32_t drv_uart_inject(int num, const uint8_t *pData, uint16_t dataLen);
int32_t drv_uart_get_charNonBlocking(int32_t drv, uint8_t *pBuff);

#endif