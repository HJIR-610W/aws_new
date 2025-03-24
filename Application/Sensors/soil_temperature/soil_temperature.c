#include <stdio.h>
#include <math.h>
#include <string.h>


#include "soil_temperature.h"
#include "driver_adc.h"
#include "app_sensor.h"
#include "dev_io.h"
#include "Sensors\general\general_adc.h"



driver_t *soilTemp_open(int32_t num,void *opt)
{
  void *driver;

  switch (num)
  {
    case GENERAL_ADC:
    driver  = general_adc_open(num,opt);
    break;
  }

  return driver;
}


float read_sensor_soilTemp(driver_t *driver,uint8_t *err)
{

  if(driver == NULL)
  {
    *err = 1;
    return NAN;
  }

  if(strncmp(driver->name,"GENERAL_ADC",11)==0)
  {
    return general_adc_read(driver,err);
  }

  return NAN;
}
