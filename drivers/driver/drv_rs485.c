
#include "drv_rs485.h"


#include "bsp_rs485.h"


#include "util_memory.h"
typedef struct app_rs485_s
{
  uint8_t num;
  const char *name;
}app_rs485_t;

const app_rs485_t rs485_define[] = {{.num = DRV_RS485_RS232_A, .name = "232/485 A"},
                                    {.num = DRV_RS485_RS232_B, .name = "232/485 B"},
                                    {.num = DRV_RS485_C, .name = "485 C"}};

bool app_rs485Open[eAPP_RS485_MAX];



int32_t rs485_num_to_driver_num(int32_t app_rs485_num)
{
 
  return rs485_define[app_rs485_num].num;
}

uint16_t drv_rs485_get_portList(const char **list,uint16_t listMax)
{
  int i=0;
  for( i = 0; i <_countof(rs485_define);i++)
  {
    if(i<listMax)
    {
      list[i] = rs485_define[i].name;
    }
  }
  return i;
}




int32_t drv_rs485_init(int32_t num,void *opt,const char *owner)
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
