
#include "bsp.h"
#include "drv_system.h"


void drv_system_init(void)
{
  
}


float drv_system_read(int num)
{
  float value=0;

  switch (num)
  {
    case DRV_SYS_BATTERY:
    value = bsp_read_battery();
      break;
    case DRV_SYS_TEMPERATURE:
      value = bsp_read_temperature();
      break;
    default:
    value = 0;
      break;
  }

  return value;
}
