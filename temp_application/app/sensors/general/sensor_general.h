
#ifndef SENSOR_GENERAL_H

#define SENSOR_GENERAL_H

#include "app_sensor.h"

#include "driver_interface.h"


#define GENERAL_ADC_S_0 0
#define GENERAL_ADC_S_1 1
#define GENERAL_ADC_S_2 2
#define GENERAL_ADC_S_3 3
#define GENERAL_ADC_S_4 4
#define GENERAL_ADC_S_5 5
#define GENERAL_ADC_S_6 6
#define GENERAL_ADC_S_7 7
#define GENERAL_ADC_S_8 8
#define GENERAL_ADC_S_9 9
#define GENERAL_ADC_S_10 10
#define GENERAL_ADC_S_11 11
#define GENERAL_ADC_S_12 12
#define GENERAL_ADC_S_13 13
#define GENERAL_ADC_S_14 14
#define GENERAL_ADC_S_15 15

#define GENERAL_ADC_D_0 16
#define GENERAL_ADC_D_1 17
#define GENERAL_ADC_D_2 18
#define GENERAL_ADC_D_3 19
#define GENERAL_ADC_D_4 20
#define GENERAL_ADC_D_5 21
#define GENERAL_ADC_D_6 22
#define GENERAL_ADC_D_7 23

#define GENERAL_RS485_A 24
#define GENERAL_RS485_B 25


void *general_sensor_open(sensor_t *sensor,uint8_t *err);

float general_sensor_read(void *driver,uint8_t *err);

#endif