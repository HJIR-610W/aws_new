

#include <math.h>
#include <string.h>

#include "Sensors\wind_speed\wind_speed.h"
#include "sensors\wind_speed\hj_wind_speed_modbus.h"
#include "Sensors\general\general_adc.h"
#include "Sensors\general\general_frequency.h"
#include "hj_wind.h"
#include "wind_spd_rmyoung_05103v.h"




driver_t * windSpeed_open(int32_t num,void *opt)
{
  driver_t *driver=NULL;

  switch (num)
  {
    case GENERAL_ADC:
    driver = general_adc_open(num,opt,"Wind Speed");
    break;
    case WIND_HJ:
    driver = hjwind_open(opt)  ;
    break;
    case GENERAL_FREQ:
      driver = general_freq_open(opt);
      break;
    case WIND_SPEED_RMYOUNG_05103V:
    wind_spd_rmyoung_05103v_init(opt);
    driver = (driver_t *)wind_spd_rmyoung_05103v_init;
    break;
    case WIND_SPEED_HJ_MODBUS:
    hj_wind_speed_init(opt);
        driver = (driver_t *)hj_wind_speed_init;
    break;

    default:
      break;
  }

  return driver;
}

float wind_read(driver_t *driver,int32_t channel,uint8_t *err)
{
  const wind_api_t *api = ((driver_t *)driver)->api;

  if(driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return NAN;
  }

  if (driver == (driver_t*)wind_spd_rmyoung_05103v_init)
  {
    return read_wind_spd_rmyoung_05103v(err);
  }
  else if(driver == (driver_t *)hj_wind_speed_init)
  {
    return hj_wind_speed_read(err);
  }

    if (strncmp(driver->name, "GENERAL_ADC", 11) == 0)
    {
      return general_adc_read(driver, err);
    }
    else if (strncmp(driver->name, "GENERAL_FREQ", 11) == 0)
    {
      return general_freq_read(driver, err);
    }


  return api->read(driver,channel,err);
}


