




#ifndef DRIVER_STM32_DI_H
#define DRIVER_STM32_DI_H

#include <stdint.h>

#include "driver_uart_def.h"
#include "driver_interface.h"

#define STM32_CDC        0

int32_t stm32_cdc_init(int num,void *opt);


int32_t stm32_cdc_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size,
                           uint32_t timeout1_ms, uint32_t timeout2_ms);

int32_t stm32_cdc_send(int num, const uint8_t *pData, uint16_t dataLen);
int32_t stm32_cdc_recv(int num, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs);
int32_t stm32_cdc_inject(int num, const uint8_t *pData, uint16_t dataLen);
void stm32_cdc_flush_rx(void);
#endif
