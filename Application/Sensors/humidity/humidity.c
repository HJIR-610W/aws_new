


#include "Sensors\humidity\humidity.h"
#include "Sensors\general\general_adc.h"

driver_t *humidity_open(int32_t num,void *opt)
{
  driver_t *driver;

  switch (num)
  {
  case GENERAL_ADC:
    driver = general_adc_open(GENERAL_ADC,opt);
    break;
  default:
    break;
  }

  return driver;
}

float read_sensor_humidity(driver_t *driver,uint8_t *err)
{

}