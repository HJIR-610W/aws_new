



#include <stdio.h>
#include <math.h>
#include <string.h>

#include "Sensors\solar_radiation\solar_radiation.h"
#include "Sensors\general\general_adc.h"
#include "driver_adc.h"
#include "app_sensor.h"

#include "solarRadiation_define.h"


driver_t *solarRadiation_open(int32_t num,void *opt)
{
  driver_t *driver;

  switch (num)
  {
  case GENERAL_ADC:
    driver = general_adc_open(GENERAL_ADC,opt);
    break;
 }

 return driver;
}

float read_sensor_solarRadiation(driver_t *driver,uint8_t *err)
{
  const solarRadiation_api_t *api = ((driver_t *)driver)->api;

  if (driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return NAN;
  }

  if(strncmp(driver->name,"GENERAL_ADC",11)==0)
  {
    return general_adc_read(driver,err);
  }


  return api->read(driver,err);
}
