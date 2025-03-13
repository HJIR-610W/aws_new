

#include <math.h>



#include "Sensors\temperature\temperature.h"
#include "Sensors\general\sensor_general.h"
#include "utile.h"
#include "pt100.h"



void *temperature_open(uint8_t num,void *opt)
{
  void *driver;

  switch (num)
  {
    case TEMP_PT100_A:
    driver  = pt100_open(PT100_A,opt);
    break;
  case TEMP_PT100_B:
    driver  = pt100_open(PT100_B,opt);
    break;

  }

  return driver;
}

float temperature_read(void *driver,uint8_t *err)
{
  const temperature_api_t *api = ((driver_t *)driver)->api;

  if(driver == NULL)
  {
    *err = 1;
    return NAN;
  }

  return api->read(driver,err);
}
