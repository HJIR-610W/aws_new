

#ifndef TLC16C554_H
#define TLC16C554_H


#include <stdint.h>


void check_baud_rate(int uart_num);
void set_baud_rate(int uart_num,uint32_t baud_rate) ;
void send_data(int uart_num,uint8_t data);
void send_data_n(int uart_num,uint8_t *p_data,uint16_t dataLen);
int read_data(int uart_num, uint8_t *data);

void init_uart(int uart_num, uint32_t baud_rate);


#endif