
#include "drv_rs485.h"
#include "drv_rs232.h"
#include "app_rs485.h"
#include "util_memory.h"
typedef struct app_rs485_s
{
  uint8_t num;
  const char *name;
}app_rs485_t;

const app_rs485_t rs485_define[] = {{.num = DRV_RS485_A, .name = "485 A"},
                                    {.num = DRV_RS485_RS232_C, .name = "232/485 A"},
                                    {.num = DRV_RS485_RS232_D, .name = "232/485 B"}};

bool app_rs485Open[eAPP_RS485_MAX];



int32_t rs485_num_to_driver_num(int32_t app_rs485_num)
{
  int32_t num = 0;

  switch (app_rs485_num)
  {     
  case eAPP_RS485_A:
    num = DRV_RS485_A;
    break;
  case eAPP_RS485_RS232_A:
    num = DRV_RS485_RS232_C;
    break;
    break;
  case eAPP_RS485_RS232_B:
    num = DRV_RS485_RS232_D;
    break;
  }

  return num;
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
