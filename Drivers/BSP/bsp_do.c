
#include "driver_do.h"

driver_t *app_do[6];

void bsp_do_init(void)
{
  app_do[0]  = driver_do_open(DO_EXT_0,0);
  app_do[1]  = driver_do_open(DO_EXT_1,0);
  app_do[2]  = driver_do_open(DO_EXT_2,0);
  app_do[3]  = driver_do_open(DO_EXT_3,0);
  app_do[4]  = driver_do_open(DO_EXT_4,0);
  app_do[5]  = driver_do_open(DO_EXT_5,0);
}


void bsp_write_do(int32_t num,int32_t status)
{
  if(status)
  {
    driver_do_high(app_do[num]);
  }
  else
  {
    driver_do_low(app_do[num]);
  }
}