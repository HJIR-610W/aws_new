#include "bsp_uart.h"
#include "drv_rs232.h"

int32_t drv_rs232_init(int32_t num, void *opt){
  return bsp_uart_init(num, opt);
}
void drv_uart_close(int num){
  bsp_uart_close(num);
}
int32_t drv_uart_send(int num, const uint8_t *pData, uint16_t dataLen){
  return bsp_uart_send(num, pData, dataLen);
}
int32_t drv_uart_recv(int num, uint8_t *pBuff, uint16_t rLen,
                      uint32_t timeOutMs){
  return bsp_uart_recv(num, pBuff, rLen, timeOutMs);
}
int32_t drv_uart_recv_crlf(int num, char *pBuff, uint16_t bSize,
                           uint32_t tout_ms){
  return bsp_uart_recv_crlf(num, pBuff, bSize, tout_ms);
}
void drv_uart_set(int num, uart_set_option_t cmd, void *para){
  bsp_uart_set(num, cmd, para);
}

// 밑에 두함수는 대체 필요
int32_t drv_uart_get_char(int num, uint8_t *pBuff, uint16_t rLen){
  return bsp_uart_get_char(num, pBuff, rLen);
}
int32_t drv_uart_get_charNonBlocking(int num, uint8_t *pBuff){
  return bsp_uart_get_charNonBlocking(num, pBuff);
}

void drv_uart_flush_rx(int num){
  bsp_uart_flush_rx(num);
}

int32_t drv_uart_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size,
                          uint32_t timeout1_ms, uint32_t timeout2_ms){
  return bsp_uart_recv_opt(num, buffer, buffer_size, timeout1_ms, timeout2_ms);
}
void drv_uart_get(int num, uart_get_option_t cmd, void *para){
  bsp_uart_get(num, cmd, para);
}
int32_t drv_uart_recv_ll(int num, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutMs){
  return bsp_uart_recv_ll(num, pBuff, rLen, timeOutMs);
}

int32_t drv_uart_inject(int num, const uint8_t *pData, uint16_t dataLen){
  return bsp_uart_inject(num, pData, dataLen);
}

