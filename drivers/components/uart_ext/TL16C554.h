

#ifndef TLC16C554_H
#define TLC16C554_H

#include <stdint.h>

#include "driver_uart_def.h"


#define TL16C554_UART_1_D_SUB   0
#define TL16C554_UART_2_TTL_TTL 1
#define TL16C554_UART_3_RS232_A 2
#define TL16C554_UART_4_RS232_B 3
#define TL16C554_UART_5_RS485_A 4
#define TL16C554_UART_6_RS485_B 5
#define TL16C554_UART_7_RS232_C 6
#define TL16C554_UART_8_RS232_D 7

#define TL16C554_UART_MAX       8


int32_t tl16c554_send(int uart_num, const uint8_t *pData, uint16_t data_len);
void tl16c554_flush_rx(int uart_num);
void tl16c554_set(int uart_num, eUART_SET_OPTION_t option, void *value);
void tl16c554_uart_get(int uart_num, eUART_GET_OPTION_t cmd, void *option);
void tl16c554_uart_set_config(int uart_num, uart_config_t *config);
int32_t tl16c554_uart_inject(int uart_num, const uint8_t *pData, uint16_t dataLen);
int32_t tl16c554_recv(int uart_num, uint8_t *p_buff, uint16_t buff_size, uint32_t timeout_ms);
int32_t tl16c554_recv_opt(int uart_num, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms, uint32_t timeout2_ms);
int32_t tl16c554_recv_ll(int uart_num, uint8_t *p_buff, uint16_t buff_size, uint32_t timeout_ms);
int32_t tl16c554_uart_recv_crlf(int uart_num, char *p_buff, uint16_t bSize, uint32_t tout_ms);
void tl16c554_close(int uart_num);
int32_t tl16c554_init(int32_t uart_num, void *opt);
#endif
