



#include <stdio.h>

#include "Sensors\sunshine\sunshine.h"
#include "driver_adc.h"
#include "app_sensor.h"

#include "dev_io.h"



bool sunShineInit=false;

void sunShine_init(sensor_t *sensor,void *opt)
{
  sunShineInit = true;
}

bool is_sunShineInit(void)
{
  return sunShineInit;
}

bool sunShine_deInit(void)
{
  sunShineInit = false;
}

float read_sensor_sunshine(sensor_t *sensor,uint8_t *err)
{
  float data;
    adc_config_t *adc;
    
  switch(sensor->type)
  {
    case S_T_ADC:

    adc = get_sensor_config(sensor);
    data = calculate_adc(adc,err);

    break;
    case S_T_GENERAL_485:
  
    break;
  }

  return data;
}