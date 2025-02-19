
#ifndef HUMIDITY_H
#define HUMIDITY_H
#include <stdint.h>
#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"


#define HUMI_ERR_VAL 1000


void humidity_init(sensor_t *sensor);
float read_sensor_humidity(sensor_t *sensor,uint8_t *err);

#endif