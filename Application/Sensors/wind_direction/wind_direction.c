

#include "Sensors\wind_direction\wind_direction.h"











float read_sensor_windDirection(sensor_t *sensor,uint8_t *err)
{
  float data;
  float gain;
  void *cfg = get_sensor_config(sensor);;

  if(cfg ==NULL)
  {
    *err = 2;
    return 0;
  }
  switch(sensor->type)
  {
    case S_T_ADC:
    data = calculate_adc(cfg,err);

    break;
    case S_T_TEMP_232:

    break;
  }

  return data;
}