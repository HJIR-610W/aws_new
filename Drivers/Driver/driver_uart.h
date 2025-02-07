
#ifndef DRIVER_UART_H
#define DRIVER_UART_H

#include "driver_interface.h"
#include "driver_uart_def.h"

#define UART_0_D_SUB_0  0 //표시기
#define UART_1_TTL      1 //블루투스 모듈 
#define UART_2_EXT3     2 //사용자0
#define UART_3_EXT4     3 //사용자1
#define UART_4_RS485_A  4 //RS485 A
#define UART_5_RS485_B  5 //RS485 B
#define UART_6_EXT1     6 //사용자2
#define UART_7_EXT2     7 //사용자3
#define UART_8_DEBUG    8 //디버깅용
#define UART_9_CDMA     9 //CDMA
#define UART_10_SDI    10 //SDI통신


#define UART_ERR_TIMEOUT -1
#define UART_ERR_SIZE    -2

driver_t *driver_uart_open(int32_t  num,void *opt);


void driver_uart_close(driver_t *drv);
int32_t driver_uart_send(driver_t *drv,const uint8_t *pData,uint16_t dataLen);
int32_t driver_uart_recv(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutMs);
int32_t driver_uart_recv_opt(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,
                                  eUART_RECV_OPT_t cmd,void *opt);
int32_t drier_uart_recv_crlf(driver_t *drv,char *pBuff,uint16_t bSize,uint32_t tout_ms);

void driver_uart_set(driver_t *uart,uart_set_option_t cmd,void *para);

//밑에 두함수는 대체 필요
int32_t driver_uart_get_char(driver_t *drv,uint8_t *pBuff,uint16_t rLen);
int32_t driver_uart_get_charNonBlocking(driver_t *drv,uint8_t *pBuff);



#endif
