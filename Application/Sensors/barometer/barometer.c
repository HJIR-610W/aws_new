

#include "config.h"
#include "app_adc.h"
#include "Sensors\barometer\barometer.h"




void barometer_init(sensor_t *sensor)
{
  
}



int32_t read_sensor_barometer(sensor_t *sensor,uint8_t *err)
{
  int32_t data=0;
    adc_config_t *adc;
  switch(sensor->type)
  {
    case S_T_ADC:

    adc = get_sensor_config(sensor);

    break;
    case S_T_TEMP_232:

    break;
  }

  return data;
}