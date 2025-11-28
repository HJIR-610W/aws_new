

#ifndef SOIL_TEMPERATURE_H
#define SOIL_TEMPERATURE_H

#include <stdint.h>
#include "driver_interface.h"
#include "config_app.h"
#include "app_sensor.h"
#include "app_adc.h"



#ifndef GENERAL_ADC
#define GENERAL_ADC   0
#endif

#ifndef GENERAL_RS485
#define GENERAL_RS485 1
#endif


driver_t *soilTemp_open(int32_t num,void *opt,const char *owner);
float read_sensor_soilTemp(driver_t *sensor,uint8_t *err);



#endif