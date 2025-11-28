

#ifndef GENERAL_ADC_H
#define GENERAL_ADC_H


#include <app_sensor.h>

#define GENERAL_ADC 0

//#define GENERAL_AD 0




//
#define GENERAL_ADC_SINGLE 0
#define GENERAL_ADC_DIFF 1

void *general_adc_open(uint8_t num,void *opt,const char *owner);
float general_adc_read(void *driver,uint8_t *err);
const char *general_adc_read_owner(int mode,int channel);




extern const char *g_adc_single_owner_list[16];
extern const char *g_adc_diff_owner_list[8];



#endif