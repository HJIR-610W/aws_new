
#ifndef WIND_SPEED_H
#define WIND_SPEED_H

#include <stdint.h>
#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"

#define WIND_SPEED_ERR_VAL 1000

void windSpeed_init(void);
float read_sensor_windSpeed(sensor_t *sensor,uint8_t *err);


extern float windSpeedSample1Min[240];
extern float windSpeedSample10Min[10];
#endif