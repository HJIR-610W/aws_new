

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <stdint.h>
#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"

float read_sensor_temperature(sensor_t *sensor,uint8_t *err);

#endif