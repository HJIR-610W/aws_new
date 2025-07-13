
#include <math.h>
#include <string.h>


#include "config_app.h"
#include "app_adc.h"
#include "Sensors\barometer\barometer.h"
#include "Sensors\general\general_adc.h"



driver_t *barometer_open(int32_t num,void *opt)
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

float read_sensor_barometer(driver_t *driver,uint8_t *err)
{

  if(driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return NAN;
  }

  if(strncmp(driver->name,"GENERAL_ADC",11)==0)
  {
    return general_adc_read(driver,err);
  }

  *err = DRV_ERR_NONE;
  
  return 0.0f;
}


