



#include <stdio.h>
#include <math.h>
#include <string.h>

#include "Sensors\sunshine\sunshine.h"
#include "Sensors\sunshine\sunshine_csd3.h"
#include "sunshine_define.h"
#include "drv_adc.h"
#include "app_sensor.h"

#include "dev_io.h"
#include "Sensors\general\general_adc.h"



bool sunShineInit=false;

void sunShine_init(sensor_t *sensor,void *opt)
{
  sunShineInit = true;
}





driver_t *sunshine_open(int32_t num,void *opt)
{
  driver_t *driver=NULL;

  switch (num)
  {
  case GENERAL_ADC:
    driver = general_adc_open(GENERAL_ADC,opt,"Sunshine");
    break;
  case SUNSHINE_CSD3:
      solar_duration_csd3_init(opt);  
      driver = (driver_t *)solar_duration_csd3_init;
     break;
  break;
 }

 return driver;
}

float read_sensor_sunshine(driver_t *driver,uint8_t *err)
{
  const sunshine_api_t *api = ((driver_t *)driver)->api;

  if (driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return NAN;
  }

  if (driver == (driver_t*)solar_duration_csd3_init)
  {
    return read_solar_duration_csd3(err);
  }



  if(strncmp(driver->name,"GENERAL_ADC",11)==0)
  {
    return general_adc_read(driver,err);
  }


  return api->read(driver,err);
}
