
#ifndef APP_ADC_H
#define APP_ADC_H

#include <stdint.h>

extern const uint8_t user_adc_single_channel[18];


void adc_init(void);
int32_t adc_read_single(int channel,uint8_t *err);
int32_t adc_read_diff(int channel,uint8_t *err);
float cvt_adcToVol(int32_t adc,int32_t off,int32_t full,int32_t off_in,int32_t full_in);
int32_t adc_read_single_avg(int channel,uint8_t *err,uint8_t avg_cnt);
int32_t adc_read_diff_avg(int channel,uint8_t *err,uint8_t avg_cnt);

float adc_chToVoltage(int32_t mode,int32_t channel,int32_t adc);
#endif