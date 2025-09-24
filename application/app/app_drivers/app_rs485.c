
#include "drv_rs485.h"
#include "drv_rs232.h"
#include "app_rs485.h"
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
