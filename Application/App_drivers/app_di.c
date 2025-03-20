

#include "driver_di.h"

driver_t *app_di[6];

void di_init(void)
{
  app_di[0] = driver_di_open(DI_EXT_0,0);
  app_di[1] = driver_di_open(DI_EXT_1,0);
  app_di[2] = driver_di_open(DI_EXT_2,0);
  app_di[3] = driver_di_open(DI_EXT_3,0);
  app_di[4] = driver_di_open(DI_EXT_4,0);
  app_di[5] = driver_di_open(DI_EXT_5,0);
}

int32_t read_di(int32_t num)
{
  return driver_di_read( app_di[num]);
}

bool is_di_pressed(int32_t num)
{
  if(read_di(num))
  {
    return false;
  }

  return true;
}




