

#include <math.h>
#include <string.h>

#include "Sensors\wind_speed\wind_speed.h"
#include "sensors\wind_speed\hj_wind.h"
#include "Sensors\general\general_adc.h"
#include "Sensors\general\general_frequency.h"
#include "Sensors\wind_speed\hj_wind.h"
#include "Sensors\wind_direction\hj_wind_direction.h"
#include "Sensors\wind_direction\wind_dir_rmyoung_05103v.h"
#include "sensors\wind_direction\wind_direction.h"
driver_t *wind_direction_open(uint8_t num, void *opt)
{
  driver_t *driver = NULL;

  switch (num)
  {
  case GENERAL_ADC:
    driver = general_adc_open(num, opt,"Wind Direction");
    break;
  case HJ_WIND_DIRECTION:
    driver = hjwind_direction_open(opt);
    break;
  case GENERAL_FREQ:
    driver = general_freq_open( opt);
    break;
  case WIND_DIRECTION_RMYOUNG_05103V:
    wind_dir_rmyoung_05103v_init(opt);
    driver = (driver_t *)wind_dir_rmyoung_05103v_init;
     break;
    default : break;
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

  if (driver == (driver_t*)wind_dir_rmyoung_05103v_init)
  {
    return read_wind_dir_rmyoung_05103v(err);
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
