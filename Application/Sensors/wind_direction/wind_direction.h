
#ifndef WIND_DIRECTION_H
#define WIND_DIRECTION_H

#include <stdint.h>
#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"

#define WIND_DIRECTION_ERR_VAL 1000




void windDirection_init(sensor_t *sensor);

bool is_windDirectionInit(void);
float read_sensor_windDirection(sensor_t *sensor,uint8_t *err);


extern float windDirectionSample1Min[240];
extern float windDirectionSample10Min[10];
extern uint16_t windDirectionSample1MinCnt;
extern uint16_t windDirectionSample10MinCnt;
#endif