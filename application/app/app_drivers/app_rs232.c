
#include "drv_rs232.h"
#include "app_rs232.h"
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
