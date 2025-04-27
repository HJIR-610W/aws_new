
#ifndef APP_RS232_H
#define APP_RS232_H

#include <stdint.h>
#include <stdbool.h>
#include "driver_uart_def.h"
typedef enum rs232_port_e
{
  eRS232_1,
  eRS232_2,
  eRS232_3,
  eRS232_4,
  eRS232_MAX
}eRS232_PORT_t;


void rs232_open(eRS232_PORT_t port,void *opt);
void rs232_close(eRS232_PORT_t port);
void rs232_set(eRS232_PORT_t port,uint32_t baud,uint8_t parity);
void rs232_send(eRS232_PORT_t port,uint8_t *pData,uint16_t dataLen);
uint16_t rs232_recv(eRS232_PORT_t port,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms);
uint16_t rs232_get_portList(const char **list,uint16_t listMax);
bool rs232_is_opened(eRS232_PORT_t port);

uint16_t rs232_recvOpt(eRS232_PORT_t port,uint8_t *pBuff,uint16_t rLen,
                      uint32_t timeOutms,uint32_t dataTimeOutms);

int32_t uart_num_to_driver_num(int32_t app_uart_num);
#endif