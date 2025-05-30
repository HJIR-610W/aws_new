




#include <stdio.h>

#include "driver_charger.h"

#include "hjsmartCharger.h"
#include "ls1024.h"

driver_t *driver_charger_open(int32_t num,void *opt)
{
  driver_t *driver = 0;

  switch (num)
  {
  case CHARGER_HJ_SMART:
    driver = hjsmartCharger_open(HJ_SMART_CHARGER,opt);
    break;
    case CHARGER_LS1024:
      driver = ls1024_open(LS1024_CHARGER, opt);
      break;
  }
  
  return driver;
}

void driver_charger_read(driver_t *driver,charger_data_t *data,uint8_t *err)
{
  const charger_api_t *api = driver->api;

  if(driver==NULL)
  {
    *err = 1;
  return;
  }
  api->read(driver,data,err);
}