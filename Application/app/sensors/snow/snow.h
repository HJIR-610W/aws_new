
#ifndef SENSOR_SNOW_H_
#define SENSOR_SNOW_H_

#include "Sensors\snow\hj_snow.h"
#include "drv_adc.h"
#include "app_sensor.h"


#ifndef GENERAL_ADC
#define GENERAL_ADC   0
#endif

#ifndef GENERAL_RS485
#define GENERAL_RS485 1
#endif

#define SNOW_HJ 100


driver_t *snow_open(int32_t num,void *opt);
int32_t read_sensor_snow(driver_t *driver,uint8_t *err);


#endif