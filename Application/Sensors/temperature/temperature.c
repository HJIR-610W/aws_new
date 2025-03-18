
#include <string.h>
#include <math.h>

#include "Sensors\temperature\temperature.h"
#include "Sensors\general\sensor_general.h"
#include "Sensors\general\general_adc.h"

#include "utile.h"
#include "pt100.h"


void *temperature_open(uint8_t num,void *opt)
{
  void *driver;


  switch (num)
  {
    case GENERAL_ADC:
    driver  = general_adc_open(num,opt);
    break;

    case TEMP_PT100_A:
    driver  = pt100_open(PT100_A,opt);
    break;
  case TEMP_PT100_B:
    driver  = pt100_open(PT100_B,opt);
    break;

  }

  return driver;
}

float temperature_read(driver_t *driver,uint8_t *err)
{
  const temperature_api_t *api = ((driver_t *)driver)->api;

  if(driver == NULL)
  {
    *err = 1;
    return NAN;
  }

  if(strncmp(driver->name,"GENERAL_ADC",11)==0)
  {
    return general_adc_read(driver,err);
  }

  
  return api->read(driver,err);
}
