
#include "driver_uart.h"
#include "app_rs232.h"
#include "utile.h"

typedef struct app_rs232_s
{
  uint8_t num;
  const char *name;
}app_rs232_t;

const app_rs232_t rs232_define[]={{.num = UART_STM32_1,   .name ="DEBUG"},
                                  {.num = UART_STM32_3,   .name ="D-SUB CDMA"},
                                  {.num = UART_EX_232_1,  .name ="D-SUB EX1 232"},
                                  {.num = UART_EX_TTL_2,  .name ="EX2 TTL"},
                                  {.num = UART_EX_232_A_3,.name ="EX3 232 A"},
                                  {.num = UART_EX_232_B_4,.name ="EX4 232 B"},
                                  {.num = UART_EX_232_C_7,.name ="EX7 232 C"},
                                  {.num = UART_EX_232_D_8,.name ="EX8 232 D"}};

driver_t *rs232_drivers[eRS232_MAX];




void rs232_open(eRS232_PORT_t port)
{
  rs232_drivers[(int)port] = driver_uart_open(rs232_define[(int)port].num);
}

void rs232_set(eRS232_PORT_t port,uint32_t baud,uint8_t parity)
{
  uart_baud_config_t uart_cfg;

  if(rs232_drivers[(int)port])
  {
    uart_cfg.baud   = baud;
    uart_cfg.parity = parity;

    driver_uart_set(rs232_drivers[(int)port],eUART_SET_CONFIG,(void *)&uart_cfg);
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

  return driver_uart_recvs(rs232_drivers[(int)port],pBuff,rLen,timeOutms);
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