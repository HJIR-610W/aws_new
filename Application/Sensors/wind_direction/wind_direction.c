

#include "Sensors\wind_direction\wind_direction.h"




float windDirectionSample1Min[240];
float windDirectionSample10Min[10];
uint16_t windDirectionSample1MinCnt;
uint16_t windDirectionSample10MinCnt;

bool windDirectionInit=false;

void windDirection_init(void)
{
  windDirectionInit = true;
}

bool is_windDirectionInit(void)
{
  return windDirectionInit;
}

bool windDirection_deInit(void)
{
  windDirectionInit = false;
}

float read_sensor_windDirection(sensor_t *sensor,uint8_t *err)
{
  float data;

  if(is_windDirectionInit()==false)
  {
    *err = 2;
    return 0;
  }

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