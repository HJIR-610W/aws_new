
#ifndef DRIVER_UART_H
#define DRIVER_UART_H

#include "driver_interface.h"
#include "driver_uart_def.h"

#define UART_0_D_SUB_0 0  // VHF
#define UART_1_TTL 1      // 블루투스 모듈
#define UART_2_EXT_A 2    // 사용자0
#define UART_3_EXT_B 3    // 사용자1
#define UART_4_EXT_C 4    // 사용자2
#define UART_5_EXT_D 5    // 사용자3
#define UART_6_RS485_A 6  // RS485 A
#define UART_7_RS485_B 7  // RS485 B
#define UART_8_CDMA 8     // CDMA
#define UART_9_SDI 9      // SDI통신
#define UART_10_CDC 10    // USB 디버깅

#define UART_ERR_TIMEOUT -1
#define UART_ERR_SIZE -2

driver_t *driver_uart_open(int32_t num, void *opt);

void driver_uart_close(driver_t *drv);
int32_t driver_uart_send(driver_t *drv, const uint8_t *pData, uint16_t dataLen);
int32_t driver_uart_recv(driver_t *drv, uint8_t *pBuff, uint16_t rLen,
                         uint32_t timeOutMs);

int32_t driver_uart_recv_crlf(driver_t *drv, char *pBuff, uint16_t bSize,
                             uint32_t tout_ms);

void driver_uart_set(driver_t *uart, uart_set_option_t cmd, void *para);

// 밑에 두함수는 대체 필요
int32_t driver_uart_get_char(driver_t *drv, uint8_t *pBuff, uint16_t rLen);
int32_t driver_uart_get_charNonBlocking(driver_t *drv, uint8_t *pBuff);

void driver_uart_flush_rx(driver_t *drv);

int32_t driver_uart_recv_opt(driver_t *drv, uint8_t *buffer, uint16_t buffer_size,
                              uint32_t timeout1_ms, uint32_t timeout2_ms);
void driver_uart_get(driver_t *drv, uart_get_option_t cmd, void *para);
int32_t driver_uart_recv_ll(driver_t *drv, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutMs);

int32_t driver_uart_inject(driver_t *drv, const uint8_t *pData, uint16_t dataLen);
#endif
