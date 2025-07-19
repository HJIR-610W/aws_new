


#include "drv_power.h"
#include "bsp_power.h"
void drv_power_init(void)
{
  bsp_power_init();
}
void drv_power_on(int num)
{
  bsp_power_on(num);
}
void drv_power_off(int num)
{
  bsp_power_off(num);
}
