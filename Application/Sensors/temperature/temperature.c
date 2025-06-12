
#include "Sensors\temperature\temperature.h"

#include <math.h>
#include <string.h>

#include "Sensors\general\general_adc.h"
#include "Sensors\general\general_virtual.h"
#include "Sensors\general\sensor_general.h"
#include "Sensors\temperature\hj_temperature.h"
#include "pt100.h"
#include "util_memory.h"

driver_t *temperature_open(uint32_t num, void *opt)
{
  driver_t *driver;

  switch (num)
  {
    case GENERAL_ADC:
      driver = general_adc_open(num, opt);
      break;
    case GENERAL_V:
      driver = general_v_open(num, opt);
      break;
    case TEMP_PT100_A:
      driver = pt100_open(PT100_A, opt);
      break;
    case TEMP_PT100_B:
      driver = pt100_open(PT100_B, opt);
      break;
    case TEMP_HJ_TEMPERATURE:
      driver = hjTemperature_open(HJ_TEMPERATURE, opt);
      break;
  }

  return driver;
}

float temperature_read(driver_t *driver, uint8_t *err)
{
  const temperature_api_t *api = ((driver_t *)driver)->api;
  
  if (driver == NULL)
  {
    *err = 1;
    return NAN;
  }

  if (strncmp(driver->name, "GENERAL_ADC", 11) == 0)
  {
    return general_adc_read(driver, err);
  }
  else if (strncmp(driver->name, "GENERAL_V", 9) == 0)
  {
    return general_v_read(driver, err);
  }

  return api->read(driver, err);
}

void temperature_set(driver_t *driver, temperature_set_option_t option, void *value)
{
  const temperature_api_t *api = ((driver_t *)driver)->api;

  api->set(driver,option,value);

}

void temperature_get(driver_t *driver, temperature_get_option_t option, void *value)
{
  const temperature_api_t *api = ((driver_t *)driver)->api;

  api->get(driver, option, value);
}