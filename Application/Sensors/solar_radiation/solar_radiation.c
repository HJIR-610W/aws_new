



#include <stdio.h>

#include "solar_radiation.h"
#include "driver_adc.h"
#include "app_sensor.h"

#include "dev_io.h"



bool solarRadiationInit=false;

void solraRadiation_init(sensor_t *sensor,void *opt)
{
  solarRadiationInit = true;
}

bool is_solraRadiationInit(void)
{
  return solarRadiationInit;
}

void solarRadiation_deInit(void)
{
  solarRadiationInit = false;
  
   
}

float read_sensor_solarRadiation(sensor_t *sensor,uint8_t *err)
{
  float data;
    adc_config_t *adc;
    
  switch(sensor->type)
  {
    case S_T_ADC:

    adc = get_sensor_config(sensor);
    data = calculate_adc(adc,err);

    break;

  }

  return data;
}