


#include "Sensors\temperature\temperature.h"






int32_t read_sensor_temperature(sensor_t *sensor,uint8_t *err)
{
  float data;
  float gain;
  switch(sensor->type)
  {
    case S_T_ADC:
    adc_config_t *adc;
    adc = get_sensor_config(sensor,S_T_ADC);
    data = calculate_adc(adc,err)*sensor->scale;

    break;
    case S_T_RS232:

    break;
  }

  return data;
}