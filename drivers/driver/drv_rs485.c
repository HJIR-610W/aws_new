

#include "bsp_rs485.h"

int32_t drv_rs485_init(int32_t num,void *opt)
{
  return bsp_rs485_init(num,opt); 
}

int32_t drv_rs485_send(int num, uint8_t *pData, uint16_t dataLen)
{
  return bsp_rs485_send(num, pData, dataLen);
}
int32_t drv_rs485_recv(int num, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutms)
{
  return bsp_rs485_recv(num, pBuff, rLen, timeOutms);
}
int32_t drv_rs485_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms, uint32_t timeout2_ms)
{
  return bsp_rs485_recv_opt(num, buffer, buffer_size, timeout1_ms, timeout2_ms);
}
void drv_rs485_set(int num, eUART_SET_OPTION_t cmd, void *option)
{
  bsp_rs485_set(num, cmd, option);
}
void drv_rs485_get(int num, eUART_GET_OPTION_t cmd, void *value)
{
  bsp_rs485_get(num, cmd, value);
}
void drv_rs485_flush_rx(int num)
{
  bsp_rs485_flush_rx(num);
}
