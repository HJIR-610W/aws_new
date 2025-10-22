

#include <math.h>
#include <string.h>

#include "Sensors\wind_speed\wind_speed.h"
#include "sensors\wind_speed\hj_wind.h"
#include "Sensors\general\general_adc.h"
#include "Sensors\general\general_frequency.h"
#include "Sensors\wind_speed\hj_wind.h"
#include "Sensors\wind_direction\hj_wind_direction.h"

driver_t *wind_direction_open(uint8_t num, void *opt)
{
  driver_t *driver = NULL;

  switch (num)
  {
  case GENERAL_ADC:
    driver = general_adc_open(num, opt);
    break;
  case HJ_WIND_DIRECTION:
    driver = hjwind_direction_open(opt);
    break;
  case GENERAL_FREQ:
    driver = general_freq_open( opt);
    break;
  default:
    break;
  }

  return driver;
}

float wind_direction_read(driver_t *driver, int32_t channel, uint8_t *err)
{
  const wind_api_t *api = ((driver_t *)driver)->api;

  if (driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return NAN;
  }

  if (strncmp(driver->name, "GENERAL_ADC", 11) == 0)
  {
    return general_adc_read(driver, err);
  }
  else if (strncmp(driver->name, "GENERAL_FREQ", 11) == 0)
  {
    return general_freq_read(driver, err);
  }

  return api->read(driver, channel, err);
}
