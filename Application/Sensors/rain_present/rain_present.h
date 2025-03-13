

#ifndef RAIN_PRESENT_H
#define RAIN_PRESENT_H

#include <stdbool.h>
#include "app_sensor.h"


void rainPresent_init(sensor_t *sensor);
bool read_sensor_rainPresent(sensor_t *sensor,uint8_t *err);
#endif
