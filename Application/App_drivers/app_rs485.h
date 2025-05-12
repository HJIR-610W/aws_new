
#ifndef APP_RS485_H
#define APP_RS485_H

#include <stdint.h>
#include <stdbool.h>

typedef enum RS485_PORT_e
{
  eAPP_RS485_A,
  eAPP_RS485_B,
  eAPP_RS485_C,
  eAPP_RS485_D,
  eAPP_RS485_MAX
} eRS485_PORT_t;

void rs485_set(eRS485_PORT_t port,uint32_t baud,uint8_t parity);
void rs485_open(eRS485_PORT_t port,void *opt);
void rs485_close(eRS485_PORT_t port);
void rs485_send(eRS485_PORT_t port,uint8_t *pData,uint16_t dataLen);
uint16_t rs485_recv(eRS485_PORT_t port,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms);
uint16_t rs485_get_portList(const char **list,uint16_t listMax);
bool rs485_is_opened(eRS485_PORT_t port);

#endif