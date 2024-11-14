
#ifndef DRIVER_UART_H
#define DRIVER_UART_H

#include "driver_interface.h"
#include "driver_uart_def.h"

#define UART_STM32_1     0  //디버깅용
#define UART_STM32_3     1  //D-SUB
#define UART_STM32_6     2  //SDI 드라이버에서 사용
#define UART_EX_232_1    3  //D-SUB
#define UART_EX_TTL_2    4  //TTL
#define UART_EX_232_A_3  5
#define UART_EX_232_B_4  6
#define UART_EX_485_1    7 //RS485 드라이버에서 사용
#define UART_EX_485_2    8 //RS485 드라이버에서 사용
#define UART_EX_232_C_7  9
#define UART_EX_232_D_8  10

#define UART_MAX 11


driver_t *driver_uart_open(int  num);
void driver_uart_send(driver_t *uart,uint8_t *pData,uint16_t dataLen);
uint16_t driver_uart_recvs(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutMs);
void driver_uart_set(driver_t *uart,eUART_SET_CMD_t cmd,void *para);
void driver_uart_get(driver_t *uart,eUART_SET_CMD_t cmd,void *config);


int32_t driver_uart_recv(driver_t *drv,uint8_t *pBuff);



#endif
