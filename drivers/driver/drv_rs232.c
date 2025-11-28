
#include "drv_rs232.h"

#include "bsp_uart.h"
#include "util_memory.h"

typedef struct app_rs232_s
{
  uint8_t num;
  const char *name;
}app_rs232_t;

const app_rs232_t rs232_define[] = {{.num = DRV_UART_2_EXT_A, .name = "232/485 A"},
                                    {.num = DRV_UART_3_EXT_B, .name = "232/485 B"},
                                    {.num = DRV_UART_4_EXT_C, .name = "232 C"}};






int32_t uart_num_to_driver_num(int32_t app_uart_num)
{

  if (app_uart_num > eRS232_MAX)
    return rs232_define[0].num;

  return rs232_define[app_uart_num].num;

}


uint16_t rs232_get_portList(const char **list,uint16_t listMax)
{
  uint32_t i=0;


  for( i = 0; i <_countof(rs232_define);i++)
  {
    if(i<listMax)
    {
      list[i] = rs232_define[i].name;
    }
    else
    {
      list[i] = 0;
    }
  }
  return i;
}



int32_t drv_uart_init(int32_t num, void *opt,const char *owner)
{
  return bsp_uart_init(num, opt);
}


void drv_uart_close(int num)
{
  bsp_uart_close(num);
}

int32_t drv_uart_send(int num, const uint8_t *pData, uint16_t dataLen)
{
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
void drv_uart_set(int num, eUART_SET_OPTION_t cmd, void *para){
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
void drv_uart_get(int num, eUART_GET_OPTION_t cmd, void *para){
  bsp_uart_get(num, cmd, para);
}
int32_t drv_uart_recv_ll(int num, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutMs){
  return bsp_uart_recv_ll(num, pBuff, rLen, timeOutMs);
}

int32_t drv_uart_inject(int num, const uint8_t *pData, uint16_t dataLen){
  return bsp_uart_inject(num, pData, dataLen);
}










