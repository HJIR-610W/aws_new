
#include "driver_uart.h"
#include "app_rs232.h"
#include "util_memory.h"

typedef struct app_rs232_s
{
  uint8_t num;
  const char *name;
}app_rs232_t;

const app_rs232_t rs232_define[] = {{.num = UART_2_EXT_A, .name = "RS232/RS485 A"},
                                    {.num = UART_3_EXT_B, .name = "RS232/RS485 B"},
                                    {.num = UART_5_EXT_D, .name = "RS232/HART D"}};

int32_t uart_num_to_driver_num(int32_t app_uart_num)
{
  int32_t num=0;

  switch (app_uart_num)
  {
    case eRS232_RS485_A:
      num = UART_2_EXT_A;
      break;
    case eRS232_RS485_B:
      num = UART_3_EXT_B;
      break;
      break;
    case eRS232_HART_D:
      num = UART_5_EXT_D;
       break;
  }

  return num;
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
