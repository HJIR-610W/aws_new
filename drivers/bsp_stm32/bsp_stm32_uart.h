

#ifndef DRIVER_STM32_UART_H
#define DRIVER_STM32_UART_H

#include "driver_interface.h"
#include "drv_uart_def.h"


#define STM32_UART_0_CDMA 0
#define STM32_UART_1_SDI  1
#define STM32_UART_MAX    2

int32_t stm32_uart_init(int num, void *opt);
void stm32_uart_deinit(int num);
void stm32_uart_flush_rx(int num);
void stm32_uart_close(int num);
void stm32_uart_set(int num, eUART_SET_OPTION_t cmd, void *option);
void stm32_uart_get(int num, eUART_GET_OPTION_t cmd, void *option);
int32_t stm32_uart_inject(int num, const uint8_t *pData, uint16_t dataLen);
int32_t stm32_uart_recv_crlf(int num, char *pBuff, uint16_t bSize, uint32_t tout_ms);
int32_t stm32_uart_recv(int num, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs);
int32_t stm32_uart_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms,
                       uint32_t timeout2_ms);

int32_t stm32_uart_send(int num, const uint8_t *pData, uint16_t dataLen);
int32_t stm32_uart_recv_ll(int num, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs);
void stm32_uart_set_config(int num, uart_config_t *config);
#endif
