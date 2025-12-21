
#include "Sensors\temperature\temperature.h"

#include <math.h>
#include <string.h>

#include "Sensors\general\general_adc.h"

#include "Sensors\temperature\hj_temperature.h"
#include "pt100.h"
#include "util_memory.h"

driver_t *temperature_open(uint32_t num, void *opt)
{
  driver_t *driver;

  switch (num)
  {
    case GENERAL_ADC:
      driver = general_adc_open(num, opt,"Temperature");
      break;
    case TEMP_PT100:
      driver = pt100_open(opt,"temperature");
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


  return api->read(driver, err);
}

void temperature_ctrl(driver_t *driver, eHJTEMPERATURE_OPT_t option, void *w_opt, void *r_opt,
                      uint8_t *err)
{
  const temperature_api_t *api = ((driver_t *)driver)->api;

  api->ctrl(driver,option,w_opt,r_opt,err);

}

