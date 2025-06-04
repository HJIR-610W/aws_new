

#include "driver_di.h"

driver_t *bsp_di[6];

void bsp_di_init(void)
{
  bsp_di[0] = driver_di_open(DI_EXT_0,0);
  bsp_di[1] = driver_di_open(DI_EXT_1,0);
  bsp_di[2] = driver_di_open(DI_EXT_2,0);
  bsp_di[3] = driver_di_open(DI_EXT_3,0);
  bsp_di[4] = driver_di_open(DI_EXT_4,0);
  bsp_di[5] = driver_di_open(DI_EXT_5,0);
}

int32_t bsp_read_di(int32_t num)
{
  return driver_di_read( bsp_di[num]);
}

bool bsp_di_pressed(int32_t num)
{
  if(bsp_read_di(num))
  {
    return false;
  }

  return true;
}




