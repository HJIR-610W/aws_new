

#include "Sensors\wind_direction\wind_direction.h"
#include "Sensors\wind_speed\hj_wind.h"


float windDirectionSample1Min[240];
float windDirectionSample10Min[10];
uint16_t windDirectionSample1MinCnt;
uint16_t windDirectionSample10MinCnt;

bool windDirectionInit=false;

void windDirection_init(sensor_t *sensor)
{
  windDirectionInit = true;
}

bool is_windDirectionInit(void)
{
  return windDirectionInit;
}

void windDirection_deInit(void)
{
  windDirectionInit = false;
}

float read_sensor_windDirection(sensor_t *sensor,uint8_t *err)
{
  float data;
  dev_io_t dev_io;

  rs485_config_t *rs485_config;



  if(is_windDirectionInit() == false)
  {
    *err = 2;
    return 0;
  }

  void *cfg = get_sensor_config(sensor);;

  if(cfg ==0)
  {
    *err = 2;
    return 0;
  }


  return data;
}