
#ifndef ADS1220_H
#define ADS1220_H

#include <stdint.h>
#define ADC_ADS1220_S_CH_0 0
#define ADC_ADS1220_S_CH_1 1
#define ADC_ADS1220_S_CH_2 2
#define ADC_ADS1220_S_CH_3 3
#define ADC_ADS1220_S_CH_4 4
#define ADC_ADS1220_S_CH_5 5
#define ADC_ADS1220_S_CH_6 6
#define ADC_ADS1220_S_CH_7 7
#define ADC_ADS1220_S_CH_8 8
#define ADC_ADS1220_S_CH_9 9
#define ADC_ADS1220_S_CH_10 10
#define ADC_ADS1220_S_CH_11 11
#define ADC_ADS1220_S_CH_12 12
#define ADC_ADS1220_S_CH_13 13
#define ADC_ADS1220_S_CH_14 14
#define ADC_ADS1220_S_CH_15 15
#define ADC_ADS1220_S_CH_16 16
#define ADC_ADS1220_S_CH_17 17

#define ADC_ADS1220_D_CH_0 0
#define ADC_ADS1220_D_CH_1 1
#define ADC_ADS1220_D_CH_2 2
#define ADC_ADS1220_D_CH_3 3
#define ADC_ADS1220_D_CH_4 4
#define ADC_ADS1220_D_CH_5 5
#define ADC_ADS1220_D_CH_6 6
#define ADC_ADS1220_D_CH_7 7

void ads1220_init(void);
int32_t ads1220_single_read(int channel, uint16_t avg, uint8_t *err);
int32_t ads1220_diff_read(int channel, uint16_t avg, uint8_t *err);

#endif
