
#include "humidity.h"

#include <math.h>
#include <string.h>


#include "Sensors\temperature\temperature_define.h"
#include "Sensors\humidity\hj_huminity.h"
#include "Sensors\general\general_adc.h"


driver_t *humidity_open(int32_t num,void *opt)
{
  driver_t *driver=0;

  switch (num)
  {
  case GENERAL_ADC:
    driver = general_adc_open(GENERAL_ADC,opt,"humidity");
    break;
  case TEMP_HJ_HUMINITY:
    driver = hjHuminity_open(HJ_HUMINITY, opt);
    break;
  }

  return driver;
}

float read_sensor_humidity(driver_t *driver,uint8_t *err)
{
  const temperature_api_t *api = ((driver_t *)driver)->api;


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



