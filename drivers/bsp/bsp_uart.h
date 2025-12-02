

#ifndef BSP_UART_H
#define BSP_UART_H

#include <stdint.h>
#include "drv_uart_def.h"

#define BSP_UART_0_D_SUB_0 0 // VHF
#define BSP_UART_1_TTL_ONLY 1     // 블루투스 모듈
#define BSP_UART_2_EXT_A 2   // 사용자0
#define BSP_UART_3_EXT_B 3   // 사용자1
#define BSP_UART_4_EXT_C 4   // 사용자2
#define BSP_UART_5_EXT_D 5   // 사용자3
#define BSP_UART_6_RS485_A_ONLY 6 // RS485 A
#define BSP_UART_7_RS485_B_ONLY 7 // RS485 B
#define BSP_UART_8_CDMA         8 // CDMA
#define BSP_UART_9_SDI_ONLY     9 // SDI통신
#define BSP_UART_10_CDC        10 // USB 디버깅


int32_t bsp_uart_init(int32_t num, void *opt);
void bsp_uart_close(int num);
int32_t bsp_uart_send(int num, const uint8_t *pData, uint16_t dataLen);
int32_t bsp_uart_recv(int num, uint8_t *pBuff, uint16_t rLen,
                         uint32_t timeOutMs);
int32_t bsp_uart_recv_crlf(int num, char *pBuff, uint16_t bSize,
                              uint32_t tout_ms);
void bsp_uart_set(int num, eUART_SET_OPTION_t cmd, void *para);

// 밑에 두함수는 대체 필요
int32_t bsp_uart_get_char(int num, uint8_t *pBuff, uint16_t rLen);
int32_t bsp_uart_get_charNonBlocking(int num, uint8_t *pBuff);

void bsp_uart_flush_rx(int num);

int32_t bsp_uart_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size,
                             uint32_t timeout1_ms, uint32_t timeout2_ms);
void bsp_uart_get(int num, eUART_GET_OPTION_t cmd, void *para);
int32_t bsp_uart_recv_ll(int num, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutMs);

int32_t bsp_uart_inject(int num, const uint8_t *pData, uint16_t dataLen);
void bsp_uart_set_config(int num, uart_config_t *config);

#endif