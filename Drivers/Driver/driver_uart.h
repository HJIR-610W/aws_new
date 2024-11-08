
#ifndef DRIVER_UART_H

#define DRIVER_UART_H


#include "driver_interface.h"
#include "driver_uart_def.h"
#define UART_STM32_1   0  //µð¹ö±ë¿ë
#define UART_STM32_3   1  //D-SUB
#define UART_EX_232_1  2  //D-SUB
#define UART_EX_TTL_2  3  //TTL
#define UART_EX_232_3  4
#define UART_EX_232_4  5
#define UART_EX_232_7  6
#define UART_EX_232_8  7







driver_t *driver_uart_open(int  num);
void driver_send_uart(driver_t *uart,uint8_t *pData,uint16_t dataLen);
void driver_recv_uart(driver_t *uart,uint8_t *pBuff,uint16_t buffSize);
void driver_set_uart(driver_t *uart,eUART_SET_CMD_t cmd,void *para);
void driver_get_uart(driver_t *uart,eUART_SET_CMD_t cmd,void *config);




#endif