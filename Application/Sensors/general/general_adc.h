

#ifndef GENERAL_ADC_H
#define GENERAL_ADC_H


#include <app_sensor.h>

#define GENERAL_ADC 0



void *general_adc_open(uint8_t num,void *opt);
float general_adc_read(void *driver,uint8_t *err);


#endif