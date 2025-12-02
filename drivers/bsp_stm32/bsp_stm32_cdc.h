




#ifndef DRIVER_STM32_DI_H
#define DRIVER_STM32_DI_H

#include <stdint.h>

#include "drv_uart_def.h"
#include "driver_interface.h"

#define STM32_CDC        0

int32_t stm32_cdc_init(void *opt);


int32_t stm32_cdc_recv_opt( uint8_t *buffer, uint16_t buffer_size,
                           uint32_t timeout1_ms, uint32_t timeout2_ms);

int32_t stm32_cdc_send( const uint8_t *pData, uint16_t dataLen);
int32_t stm32_cdc_recv( uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs);
int32_t stm32_cdc_inject( const uint8_t *pData, uint16_t dataLen);
void stm32_cdc_flush_rx(void);
int32_t stm32_cdc_recv_crlf( char *pBuff, uint16_t bSize, uint32_t tout_ms);
#endif
