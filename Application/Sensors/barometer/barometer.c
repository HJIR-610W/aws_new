

#include "config.h"
#include "app_adc.h"
#include "Sensors\barometer\barometer.h"





int32_t read_sensor_barometer(uint8_t type,sensor_t *sensor,uint8_t *err)
{
  int32_t data;
  switch(type)
  {
    case S_T_ADC:
    adc_config_t *adc;
    adc = get_sensor_config(sensor,S_T_ADC);

    break;
    case S_T_TEMP_232:

    break;
  }

  return data;
}