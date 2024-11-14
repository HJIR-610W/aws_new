

#ifndef TLC16C554_H
#define TLC16C554_H


#include <stdint.h>
#include "driver_interface.h"
#include "driver_digitalIn.h"
#include "driver_uart_def.h"




void check_baud_rate(int uart_num);
void set_baud_rate(int uart_num,uint32_t baud_rate) ;
void send_data(int uart_num,uint8_t data);
void send_data_n(int uart_num,uint8_t *p_data,uint16_t dataLen);
int read_data(int uart_num, uint8_t *data);

void init_uart(int uart_num);




typedef struct tls16c554_s
{
  driver_t *sram_io;
  driver_t *irq_io;
}tls16c554_t;


typedef eUART_SET_CMD_t eTLS16C554_CMD_t;





driver_t *tls16c554_open(int num);
void tls16c554_send(driver_t *tls16c554,uint8_t *pData,uint16_t dataLen);
int32_t tls16c554_recv(driver_t *tls16c554,uint8_t *pData);

uint16_t tls16c554_uart_recvs(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,uint32_t timeOutMs);
int tls16c554_recv_byte(driver_t *tls16c554,uint8_t *data);
void tls16c554_set(driver_t *tls16c554,eTLS16C554_CMD_t cmd,void *option);
void tls16c554_init(driver_t *tls16c554);
#endif
