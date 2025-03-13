

#ifndef SOLAR_RADIATION_H
#define SOLAR_RADIATION_H

#include <stdint.h>
#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"


void solraRadiation_init(sensor_t *sensor,void *opt);
bool is_solraRadiationInit(void);
void solraRadiation_deInit(void);
float read_sensor_solraRadiation(sensor_t *sensor,uint8_t *err);
#endif