

#ifndef SUNSHINE_H
#define SUNSHINE_H

#include <stdint.h>
#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"


void sunShine_init(sensor_t *sensor,void *opt);
bool is_sunShineInit(void);
bool sunShine_deInit(void);
float read_sensor_sunshine(sensor_t *sensor,uint8_t *err);
#endif