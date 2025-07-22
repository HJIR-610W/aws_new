
#ifndef BAROMETER_H
#define BAROMETER_H


#include "driver_interface.h"

#define BAROMETER_ERR_VAL 1000


#ifndef GENERAL_ADC
#define GENERAL_ADC   0
#endif


#define BAROMETER_RM0 100
#define BARO_JINSUNG_SJGP215 101

driver_t *barometer_open(int32_t num,void *opt);

float read_sensor_barometer(driver_t *driver,uint8_t *err);
#endif