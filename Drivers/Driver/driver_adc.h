

#ifndef DRIVER_ADC_H
#define DRIVER_ADC_H

#include "driver_interface.h"

#define ADC_ADS1220 0
#define ADC_STM32   1


#define ADC_ADS1220_SINGLE_CH_0 0
#define ADC_ADS1220_SINGLE_CH_1 1
#define ADC_ADS1220_SINGLE_CH_2 2
#define ADC_ADS1220_SINGLE_CH_3 3//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_4 4//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_5 5
#define ADC_ADS1220_SINGLE_CH_6 6
#define ADC_ADS1220_SINGLE_CH_7 7
#define ADC_ADS1220_SINGLE_CH_8 8
#define ADC_ADS1220_SINGLE_CH_9 9
#define ADC_ADS1220_SINGLE_CH_10 10//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_11 11//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_12 12
#define ADC_ADS1220_SINGLE_CH_13 13
#define ADC_ADS1220_SINGLE_CH_14 14//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_15 15//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_16 16
#define ADC_ADS1220_SINGLE_CH_17 17
#define ADC_ADS1220_SINGLE_CH_18 18//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_19 19//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_20 20
#define ADC_ADS1220_SINGLE_CH_21 21
#define ADC_ADS1220_SINGLE_CH_22 22//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_23 23//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_24 24
#define ADC_ADS1220_SINGLE_CH_25 25
#define ADC_ADS1220_SINGLE_CH_26 26//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_27 27//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_28 28
#define ADC_ADS1220_SINGLE_CH_29 29
#define ADC_ADS1220_SINGLE_CH_30 30//하드웨어 미지원
#define ADC_ADS1220_SINGLE_CH_31 31//하드웨어 미지원


#define ADC_ADS1220_DIFF_CH_0 32
#define ADC_ADS1220_DIFF_CH_1 33
#define ADC_ADS1220_DIFF_CH_2 34
#define ADC_ADS1220_DIFF_CH_3 35
#define ADC_ADS1220_DIFF_CH_4 36
#define ADC_ADS1220_DIFF_CH_5 37
#define ADC_ADS1220_DIFF_CH_6 38
#define ADC_ADS1220_DIFF_CH_7 39

#define ADC_ADS1220_DIFF_CH_8  40  //하드웨어 미지원
#define ADC_ADS1220_DIFF_CH_9  41 //하드웨어 미지원
#define ADC_ADS1220_DIFF_CH_10 42 //하드웨어 미지원
#define ADC_ADS1220_DIFF_CH_11 43 //하드웨어 미지원
#define ADC_ADS1220_DIFF_CH_12 44 //하드웨어 미지원
#define ADC_ADS1220_DIFF_CH_13 44 //하드웨어 미지원
#define ADC_ADS1220_DIFF_CH_14 45 //하드웨어 미지원
#define ADC_ADS1220_DIFF_CH_15 46 //하드웨어 미지원



driver_t * driver_adc_open(uint32_t num);
void driver_adc_read(driver_t *adc,uint32_t *val,uint32_t ch);

#endif
