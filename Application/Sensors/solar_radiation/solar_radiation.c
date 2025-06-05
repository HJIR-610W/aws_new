



#include <stdio.h>
#include <math.h>
#include <string.h>

#include "Sensors\solar_radiation\solar_radiation.h"
#include "Sensors\general\general_adc.h"
#include "driver_adc.h"
#include "app_sensor.h"

#include "solarRadiation_define.h"
#include "ott_smp3.h"

driver_t *solar_radiation_open(int32_t num,void *opt)
{
  driver_t *driver=NULL;

  switch (num)
  {
    case GENERAL_ADC:
      driver = general_adc_open(GENERAL_ADC,opt);
    break;
    case OTT_SMP3_MODBUS:
      driver = ott_smp3_open(OTT_SMP3, opt);
    break;
    default:
    break;
  }
  
  return driver;
}

float read_sensor_solarRadiation(driver_t *driver,uint8_t *err)
{
  const solarRadiation_api_t *api;

  if (driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return NAN;
  }

  if(strncmp(driver->name,"GENERAL_ADC",11)==0)
  {
    return general_adc_read(driver,err);
  }

  api = ((driver_t *)driver)->api;

  if (api == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return NAN;
  }

  return api->read(driver,err);
}
