

#include "Sensors\wind_speed\wind_speed.h"
#include "hj_wind.h"

#include <math.h>


driver_t * windSpeed_open(uint8_t num,void *opt)
{
  driver_t *driver=NULL;

  switch (num)
  {
    case HJ_WIND:
    driver = hjwind_open(HJ_WIND,opt)  ;
    break;
  default:
    break;
  }

  return driver;
}

float wind_read(void *driver,int32_t channel,uint8_t *err)
{
  const wind_api_t *api = ((driver_t *)driver)->api;

  if(driver == NULL)
  {
    *err = 1;
    return NAN;
  }

  return api->read(driver,channel,err);
}


