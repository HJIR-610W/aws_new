
#ifndef SENSOR_SNOW_H_
#define SENSOR_SNOW_H_

#include "Sensors\snow\hj_snow.h"
#include "app_sensor.h"


void snow_init(sensor_t *sensor);

int32_t read_sensor_snow(sensor_t *sensor,uint8_t *err);
#endif