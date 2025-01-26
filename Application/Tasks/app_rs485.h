
#ifndef APP_RS485_H
#define APP_RS485_H

#include <stdint.h>


typedef enum RS485_PORT_e
{
  eAPP_RS485_A,
  eAPP_RS485_B,
  eAPP_RS485_MAX
}eRS485_PORT_t;


void rs485_open(eRS485_PORT_t port);
void rs485_close(eRS485_PORT_t port);
void rs485_sends(eRS485_PORT_t port,uint8_t *pData,uint16_t dataLen);
uint16_t rs485_recv(eRS485_PORT_t port,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms);
uint16_t rs485_get_portList(const char ***list);
bool rs485_opened(eRS485_PORT_t port);

#endif