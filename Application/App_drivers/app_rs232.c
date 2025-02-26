
#include "driver_uart.h"
#include "app_rs232.h"
#include "utile.h"

typedef struct app_rs232_s
{
  uint8_t num;
  const char *name;
}app_rs232_t;

const app_rs232_t rs232_define[]={{.num = UART_4_EXT_C,.name ="EX3_232_A"},
                                  {.num = UART_5_EXT_D,.name ="EX4_232_B"},
                                  {.num = UART_2_EXT_A,.name ="EX7_232_C"},
                                  {.num = UART_3_EXT_B,.name ="EX8_232_D"}};

driver_t *rs232_drivers[eRS232_MAX];


void rs232_open(eRS232_PORT_t port,void *opt)
{
  rs232_drivers[(int)port] = driver_uart_open(rs232_define[(int)port].num,opt);
}

void rs232_set(eRS232_PORT_t port,uint32_t baud,uint8_t parity)
{
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  if(rs232_drivers[(int)port])
  {
    uart_config.baud   = baud;
    uart_config.parityIdx = parity;

    driver_uart_set(rs232_drivers[(int)port],UART_SET_BAUDRATE,(void *)&uart_config);
  }
}

void rs232_close(eRS232_PORT_t port)
{
  //driver 해제 구현
  rs232_drivers[(int)port] = 0;
}

void rs232_send(eRS232_PORT_t port,uint8_t *pData,uint16_t dataLen)
{
  if(rs232_drivers[(int)port])
  {
    driver_uart_send(rs232_drivers[(int)port],pData,dataLen);
  }
}

uint16_t rs232_recv(eRS232_PORT_t port,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms)
{
  if(rs232_drivers[(int)port] == 0)
  {
    return 0;
  }

  return driver_uart_recv(rs232_drivers[(int)port],pBuff,rLen,timeOutms);
}



uint16_t rs232_recvOpt(eRS232_PORT_t port,uint8_t *pBuff,uint16_t rLen,
                      uint32_t timeOutms,uint32_t dataTimeOutms)
{
  if(rs232_drivers[(int)port] == 0)
  {
    return 0;
  }

//  return driver_uart_recvOpt(rs232_drivers[(int)port],pBuff,rLen,timeOutms,dataTimeOutms);
  
  return 0;
}




uint16_t rs232_get_portList(const char **list,uint16_t listMax)
{
  int i=0;
  for( i = 0; i <_countof(rs232_define);i++)
  {
    if(i<listMax)
    {
      list[i] = rs232_define[i].name;
    }
  }
  return i;
}

bool rs232_is_opened(eRS232_PORT_t port)
{
  if(rs232_drivers[(int)port] != 0)
  {
    return true;
  }
  return false;
}