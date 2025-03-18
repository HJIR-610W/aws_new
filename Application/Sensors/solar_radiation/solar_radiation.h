

#ifndef SOLAR_RADIATION_H
#define SOLAR_RADIATION_H


#include <stdint.h>
#include "config.h"

#include "app_sensor.h"
#include "app_adc.h"
#include "driver_interface.h"

#ifndef GENERAL_ADC
#define GENERAL_ADC   0
#endif

#ifndef GENERAL_RS485
#define GENERAL_RS485 1
#endif

#define SUNSHINE_A  100

driver_t *solarRadiation_open(int32_t num,void *opt);
float read_sensor_solarRadiation(driver_t *driver,uint8_t *err);


#endif