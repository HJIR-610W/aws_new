
#ifndef RAIN_H
#define RAIN_H

#include <stdbool.h>
#include "app_sensor.h"

void rain_init(sensor_t *sensor);
int32_t read_sensor_rain(sensor_t *sensor,uint8_t *err);

void rainPresent_init(void);
bool is_rainPresentInit(void);
bool rainPresent_deInit(void);

bool read_sensor_rainPresent(sensor_t *sensor,uint8_t *err);
#endif