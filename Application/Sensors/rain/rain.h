
#ifndef RAIN_H
#define RAIN_H

#include "app_sensor.h"

void rain_init(sensor_t *sensor);
int32_t read_sensor_rain(sensor_t *sensor,uint8_t *err);
#endif