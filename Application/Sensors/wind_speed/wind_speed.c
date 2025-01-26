

#include "Sensors\wind_speed\wind_speed.h"










float read_sensor_windSpeed(sensor_t *sensor,uint8_t *err)
{
  float data;
  float gain;
  switch(sensor->type)
  {
    case S_T_ADC:
    adc_config_t *adc;
    adc = get_sensor_config(sensor,S_T_ADC);
    data = calculate_adc(adc,err);

    break;
    case S_T_TEMP_232:
  
    break;
  }

  return data;
}