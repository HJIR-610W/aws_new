

#include "Sensors\wind_speed\wind_speed.h"










float read_sensor_windSpeed(sensor_t *sensor,uint8_t *err)
{
  float data;
    adc_config_t *adc;
    
  switch(sensor->type)
  {
    case S_T_ADC:

    adc = get_sensor_config(sensor);
    data = calculate_adc(adc,err);

    break;
    case S_T_TEMP_232:
  
    break;
  }

  return data;
}