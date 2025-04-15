
#include <math.h>
#include <string.h>

#include "Sensors\humidity\humidity.h"
#include "Sensors\general\general_adc.h"
#include "Sensors\general\general_virtual.h"



driver_t *humidity_open(int32_t num,void *opt)
{
  driver_t *driver=0;

  switch (num)
  {
  case GENERAL_ADC:
    driver = general_adc_open(GENERAL_ADC,opt);
    break;
  }

  return driver;
}

float read_sensor_humidity(driver_t *driver,uint8_t *err)
{
  if (driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return NAN;
  }

  if(strncmp(driver->name,"GENERAL_ADC",11)==0)
  {
    return general_adc_read(driver,err);
  }

  return NAN;
}
