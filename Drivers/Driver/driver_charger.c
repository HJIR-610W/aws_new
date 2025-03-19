




#include <stdio.h>

#include "driver_charger.h"

#include "hjsmartCharger.h"


driver_t *driver_charger_open(int32_t num,void *opt)
{
  driver_t *driver = 0;

  switch (num)
  {
  case CHARGER_HJ_SMART:
    driver = hjsmartCharger_open(HJ_SMART_CHARGER,opt);
    break;
  }
  
  return driver;
}

int32_t driver_charger_read(driver_t *driver,charger_data_t *data,uint8_t *err)
{
  const charger_api_t *api = driver->api;

  if(driver==NULL)
  {
    *err = 1;
    return 0;
  }
  return api->read(driver,data,err);
}