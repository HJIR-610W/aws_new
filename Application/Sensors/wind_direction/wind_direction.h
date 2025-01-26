
#ifndef WIND_DIRECTION_H
#define WIND_DIRECTION_H

#include <stdint.h>
#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"

float read_sensor_windDirection(sensor_t *sensor,uint8_t *err);

#endif