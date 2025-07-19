

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

