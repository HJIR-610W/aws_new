
#ifndef WIND_SPEED_H
#define WIND_SPEED_H

#include <stdint.h>
#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"


#define WIND_SPEED_ERR_VAL 1000



#ifndef GENERAL_ADC
#define GENERAL_ADC   0
#endif
#ifndef GENERAL_RS485
#define GENERAL_RS485 1
#endif

#define WIND_HJ 100


#define WIND_CHANNEL_SPEED     1
#define WIND_CHANNEL_DIRECTION 2


driver_t * windSpeed_open(uint8_t num,void *opt);
float wind_read(driver_t *driver,int32_t channel,uint8_t *err);



#endif