
#ifndef APP_ADC_H
#define APP_ADC_H

#include <stdint.h>
#include "app_sensor.h"



typedef enum adc_single_ch_e
{
  eADC_S_CH_0,
  eADC_S_CH_1,
  eADC_S_CH_2,
  eADC_S_CH_3,
  eADC_S_CH_4,
  eADC_S_CH_5,
  eADC_S_CH_6,
  eADC_S_CH_7,
  eADC_S_CH_8,
  eADC_S_CH_9,
  eADC_S_CH_10,
  eADC_S_CH_11,
  eADC_S_CH_12,
  eADC_S_CH_13,
  eADC_S_CH_14,
  eADC_S_CH_15,
  eADC_S_CH_16,//PT100_A
  eADC_S_CH_17,//PT100_B
}eADC_S_CH_t;


typedef enum adc_diff_ch_e
{
  eADC_D_CH_0,
  eADC_D_CH_1,
  eADC_D_CH_2,
  eADC_D_CH_3,
  eADC_D_CH_4,
  eADC_D_CH_5,
  eADC_D_CH_6,
  eADC_D_CH_7,
}eADC_D_CH_t;



void adc_init(void);
int32_t adc_read_single(int channel,uint8_t *err);
int32_t adc_read_diff(int channel,uint8_t *err);
float cvt_adcToVol(int32_t adc,int32_t off,int32_t full,int32_t off_in,int32_t full_in);
int32_t adc_read_single_avg(int channel,uint8_t *err,uint8_t avg_cnt);
int32_t adc_read_diff_avg(int channel,uint8_t *err,uint8_t avg_cnt);

float adc_chToVoltage(int32_t mode,int32_t channel,int32_t adc);
float adc_read_volate(adc_config_t *adc,uint8_t *err);
int32_t get_adc_vref(adc_config_t *adc);
float calculate_adc(adc_config_t *adc_config,uint8_t *err);

int32_t get_adc_single_offset(int channel);
int32_t get_adc_single_fullset(int channel);


float adc_read_volate_single(int32_t ch,uint8_t *err);
float calculate_voltage(adc_config_t *adc_config,uint8_t *err);

#endif