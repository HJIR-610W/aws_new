#include "bsp_do.h"
#include "drv_do.h"

void drv_do_init(void)
{
  bsp_do_init();
}


void drv_do_low(int num)
{
  bsp_do_low(num);
}

void drv_do_high(int num)
{
  bsp_do_high(num);
}