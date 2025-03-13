

#ifndef GENERAL_ADC_H
#define GENERAL_ADC_H


#include <app_sensor.h>

#define GENERAL_ADC_S_0 0
#define GENERAL_ADC_S_1 0
#define GENERAL_ADC_S_2 0
#define GENERAL_ADC_S_3 0
#define GENERAL_ADC_S_4 0
#define GENERAL_ADC_S_5 0
#define GENERAL_ADC_S_6 0
#define GENERAL_ADC_S_7 0
#define GENERAL_ADC_S_8 0
#define GENERAL_ADC_S_9 0
#define GENERAL_ADC_S_10 0
#define GENERAL_ADC_S_11 0
#define GENERAL_ADC_S_12 0
#define GENERAL_ADC_S_13 0
#define GENERAL_ADC_S_14 0
#define GENERAL_ADC_S_15 0

#define GENERAL_ADC_D_0 0
#define GENERAL_ADC_D_1 1
#define GENERAL_ADC_D_2 2
#define GENERAL_ADC_D_3 3
#define GENERAL_ADC_D_4 4
#define GENERAL_ADC_D_5 5
#define GENERAL_ADC_D_6 6
#define GENERAL_ADC_D_7 7

void *general_adc_single_open(uint8_t num,void *opt);
void *general_adc_diff_open(uint8_t num,void *opt);
float general_adc_read_single(void *driver,uint8_t *err);
float general_adc_read_diff(void *driver,uint8_t *err);

#endif