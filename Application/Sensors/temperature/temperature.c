


#include "Sensors\temperature\temperature.h"

#include "utile.h"


bool sensorTempInit=false;

void temperature_init(void)
{
  sensorTempInit = true;
}

bool is_sensorTempInit(void)
{
  return sensorTempInit;
}

float read_sensor_temperature(sensor_t *sensor,uint8_t *err)
{
  float data;
  void *cfg;

  if(sensorTempInit==false)
  {
    *err = 2;
    return 0;
  }
  
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
    case S_T_PT100:
    
    break;
  }

  if(*err)
  {
    data = TEMP_ERR_VAL;
  }
  return data;
}