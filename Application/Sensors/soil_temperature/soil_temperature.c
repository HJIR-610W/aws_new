#include <stdio.h>

#include "Sensors\soil_temperature\soil_temperature.h"
#include "driver_adc.h"
#include "app_sensor.h"
#include "dev_io.h"


bool soilTempInit=false;

void soilTmep_init(sensor_t *sensor,void *opt)
{
  soilTempInit = true;
}

bool is_soilTmepInit(void)
{
  return soilTempInit;
}

bool soilTmep_deInit(void)
{
  soilTempInit = false;
}

float read_sensor_soilTemp(sensor_t *sensor,uint8_t meter,uint8_t *err)
{
  float data;

  switch(meter)
  {
    case SOIL_TEMP_5CM:
    break;
    case SOIL_TEMP_10CM:
    break;
    case SOIL_TEMP_20CM:
    break;
    case SOIL_TEMP_30CM:
    break;
    case SOIL_TEMP_50CM:
    break;
    case SOIL_TEMP_100CM:
    break;
    case SOIL_TEMP_150CM:
    break;
    case SOIL_TEMP_300CM:
    break;
    case SOIL_TEMP_500CM:
    break;
  }

  return data;
}