

#ifndef DRIVER_STM32_UART_H
#define DRIVER_STM32_UART_H

#include "driver_uart_def.h"
#include "driver_interface.h"


#define STM32_UART_1 0
#define STM32_UART_3 1 
#define STM32_UART_6 2


driver_t *stm32_uart_open(int num);
void stm32_uart_send(driver_t *tls16c554,const uint8_t *pData,uint16_t dataLen);
uint16_t stm32_uart_recv(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,uint32_t timeOutMs);
void stm32_uart_set(driver_t *tls16c554,eUART_SET_CMD_t cmd,void *option);
int stm32_uart_recv_byte(driver_t *drv,uint8_t *data,uint32_t timeOutms);
void stm32_uart_init(driver_t *tls16c554);



#endif
