

#include <stdio.h>

#include "app_sensor.h"
#include "app_adc.h"

void sensorGeneral_init(sensor_t *sensor)
{

}




float read_sensorGeneral(sensor_t *sensor,uint8_t *err)
{
  float data;
  float gain;
  void *cfg = get_sensor_config(sensor);

  if(cfg == NULL)
  {
    *err = 2;
    return 0;
  }

  switch(sensor->type)
  {
    case S_T_ADC:
    data = calculate_adc(cfg,err);
    break;
    case S_T_GENERAL_232:
    break;
    case S_T_GENERAL_485:
    break;
  }

  return data;
}