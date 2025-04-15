
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "app_sensor.h"
#include "hj_snow.h"
#include "snow.h"
#include "snow_define.h"


driver_t *snow_open(int32_t num,void *opt)
{
  driver_t *driver;

  switch (num)
  {
    case SNOW_HJ_485:
    driver = hjsnow_open(HJ_SNOW_485,opt);
    break;
    case SNOW_HJ_232:
    driver = hjsnow_open(HJ_SNOW_232,opt);
    break;
  }

  return driver;
}

int32_t read_sensor_snow(driver_t *driver,uint8_t *err)
{
  int32_t data;
  const snow_api_t *api = driver->api;

  if (driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return 0;
  }

  return api->read(driver,err);

}