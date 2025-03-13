

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <stdint.h>

#include "config.h"
#include "app_sensor.h"
#include "app_adc.h"
#include "temperature_define.h"

#define TEMP_ERR_VAL 1000


#define TEMP_PT100_A              1
#define TEMP_PT100_B              2

#define TEMP_GENERAL_ADC_SINGLE_0  3
#define TEMP_GENERAL_ADC_SINGLE_1  4
#define TEMP_GENERAL_ADC_SINGLE_2  5
#define TEMP_GENERAL_ADC_SINGLE_3  6
#define TEMP_GENERAL_ADC_SINGLE_4  7
#define TEMP_GENERAL_ADC_SINGLE_5  8
#define TEMP_GENERAL_ADC_SINGLE_6  9
#define TEMP_GENERAL_ADC_SINGLE_7  10
#define TEMP_GENERAL_ADC_SINGLE_8  11
#define TEMP_GENERAL_ADC_SINGLE_9  12
#define TEMP_GENERAL_ADC_SINGLE_10  13
#define TEMP_GENERAL_ADC_SINGLE_11  14
#define TEMP_GENERAL_ADC_SINGLE_12  15
#define TEMP_GENERAL_ADC_SINGLE_13  16
#define TEMP_GENERAL_ADC_SINGLE_14  17
#define TEMP_GENERAL_ADC_SINGLE_15  18

#define TEMP_GENERAL_ADC_DIFF_0  19
#define TEMP_GENERAL_ADC_DIFF_1  20
#define TEMP_GENERAL_ADC_DIFF_2  21
#define TEMP_GENERAL_ADC_DIFF_3  22
#define TEMP_GENERAL_ADC_DIFF_4  23
#define TEMP_GENERAL_ADC_DIFF_5  24
#define TEMP_GENERAL_ADC_DIFF_6  25
#define TEMP_GENERAL_ADC_DIFF_7  26






void *temperature_open(uint8_t num,void *opt);
float temperature_read(void *driver,uint8_t *err);

#endif