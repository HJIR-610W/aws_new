

#ifndef TLC16C554_H
#define TLC16C554_H


#include <stdint.h>
#include "driver_interface.h"
#include "driver_uart_def.h"

//항상 0부터 시작해야함
#define TL16C554_UART_1_D_SUB   0
#define TL16C554_UART_2_TTL_TTL 1
#define TL16C554_UART_3_RS232_A 2
#define TL16C554_UART_4_RS232_B 3
#define TL16C554_UART_5_RS485_A 4
#define TL16C554_UART_6_RS485_B 5
#define TL16C554_UART_7_RS232_C 6
#define TL16C554_UART_8_RS232_D 7

#define TL16C554_UART_MAX 8


int32_t tls16c554_init(int32_t num, void *opt);
void tls16c554_close(int num);
int32_t tls16c554_send(int num, const uint8_t *pData, uint16_t dataLen);
int32_t tls16c554_recv(int num, uint8_t *buffer, uint16_t length, uint32_t timeOutMs);
int32_t tls16c554_recv_ll(int num, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs);
void tls16c554_flush_rx(int num);
void tls16c554_set(int num, uart_set_option_t option, void *value);
void tls16c554_uart_get(int num, uart_get_option_t cmd, void *option);

int32_t tls16c554_recv_opt(int uart_num, uint8_t *buffer, uint16_t buffer_size,
                           uint32_t timeout1_ms, uint32_t timeout2_ms);

int32_t tls16c554_uart_inject(int num, const uint8_t *pData, uint16_t dataLen);

int32_t tls16c554_uart_recv_crlf(int num, char *pBuff, uint16_t bSize, uint32_t tout_ms);
#endif
