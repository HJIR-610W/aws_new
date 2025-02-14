

#ifndef SOIL_TEMPERATURE_H
#define SOIL_TEMPERATURE_H

#include <stdint.h>
#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"


#define SOIL_TEMP_5CM   0
#define SOIL_TEMP_10CM  1
#define SOIL_TEMP_20CM  2
#define SOIL_TEMP_30CM  3
#define SOIL_TEMP_50CM  4
#define SOIL_TEMP_100CM 5
#define SOIL_TEMP_150CM 6
#define SOIL_TEMP_300CM 7
#define SOIL_TEMP_500CM 8

void soilTmep_init(sensor_t *sensor,void *opt);
bool is_soilTmepInit(void);
bool soilTmep_deInit(void);
float read_sensor_soilTemp(sensor_t *sensor,uint8_t meter,uint8_t *err);

#endif