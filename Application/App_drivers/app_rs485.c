
#include "driver_485.h"
#include "driver_uart.h"
#include "app_rs485.h"
#include "utile.h"
typedef struct app_rs485_s
{
  uint8_t num;
  const char *name;
}app_rs485_t;

const app_rs485_t rs485_define[] = {{.num = RS485_A, .name = "RS485 A"},
                                    {.num = RS485_B, .name = "RS485 B"},
                                    {.num = RS485_C, .name = "RS232/RS485 A"},
                                    {.num = RS485_D, .name = "RS232/RS485 B"}};

bool app_rs485Open[eAPP_RS485_MAX];

driver_t *rs485_drivers[eAPP_RS485_MAX];

void rs485_open(eRS485_PORT_t port,void *opt)
{
  rs485_drivers[(int)port] = driver_rs485_open(rs485_define[(int)port].num,opt);
}

void rs485_set(eRS485_PORT_t port,uint32_t baud,uint8_t parity)
{
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  if(rs485_drivers[(int)port])
  {
    
    uart_config.baud   = baud==0?19200:baud;
    uart_config.parityIdx = parity;


    driver_rs485_set(rs485_drivers[(int)port],eUART_SET_CONFIG,(void *)&uart_config);
  }
}


void rs485_close(eRS485_PORT_t port)
{

  app_rs485Open[(int)port] = 0;
}

void rs485_send(eRS485_PORT_t port,uint8_t *pData,uint16_t dataLen)
{
  driver_rs485_send(rs485_drivers[(int)port],pData,dataLen);
}

uint16_t rs485_recv(eRS485_PORT_t port,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms)
{
  return driver_rs485_recv(rs485_drivers[(int)port],pBuff,rLen,timeOutms);
}



uint16_t rs485_get_portList(const char **list,uint16_t listMax)
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

bool rs485_is_opened(eRS485_PORT_t port)
{
  return   app_rs485Open[(int)port];
}