


#include "Sensors\temperature\temperature.h"



void temperature_init(sensor_t *sensor)
{
  
}



float read_sensor_temperature(sensor_t *sensor,uint8_t *err)
{
  float data;

  void *cfg;

  cfg =  get_sensor_config(sensor);

  if(cfg==NULL)
  {
    *err = SENSOR_ERR_CFG;
    return 0;
  }
  switch(sensor->type)
  {
    case S_T_ADC:
    data = calculate_adc((adc_config_t*)cfg,err);
    break;
    case S_T_TEMP_232:

    break;
  }

  return data;
}