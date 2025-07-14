
#include "driver_485.h"
#include "driver_uart.h"
#include "app_rs485.h"
#include "util_memory.h"
typedef struct app_rs485_s
{
  uint8_t num;
  const char *name;
}app_rs485_t;

const app_rs485_t rs485_define[] = {{.num = RS485_A, .name = "485 A"},
                                    {.num = RS485_B, .name = "485 B"},
                                    {.num = RS485_C, .name = "232/485 A"},
                                    {.num = RS485_D, .name = "232/485 B"}};

bool app_rs485Open[eAPP_RS485_MAX];

driver_t *rs485_drivers[eAPP_RS485_MAX];








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
