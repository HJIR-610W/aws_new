

#ifndef DRV_ADC_H
#define DRV_ADC_H

#include <stdint.h>
#include "driver_adc_define.h"

#define DRV_ADS1220_S_CH_0 0
#define DRV_ADS1220_S_CH_1 1
#define DRV_ADS1220_S_CH_2 2
#define DRV_ADS1220_S_CH_3 3
#define DRV_ADS1220_S_CH_4 4
#define DRV_ADS1220_S_CH_5 5
#define DRV_ADS1220_S_CH_6 6
#define DRV_ADS1220_S_CH_7 7
#define DRV_ADS1220_S_CH_8 8
#define DRV_ADS1220_S_CH_9 9
#define DRV_ADS1220_S_CH_10 10
#define DRV_ADS1220_S_CH_11 11
#define DRV_ADS1220_S_CH_12 12
#define DRV_ADS1220_S_CH_13 13
#define DRV_ADS1220_S_CH_14 14
#define DRV_ADS1220_S_CH_15 15
#define DRV_ADS1220_S_CH_16 16  // PT100_A
#define DRV_ADS1220_S_CH_17 17  // PT100_B

#define DRV_ADS1220_D_CH_0 0
#define DRV_ADS1220_D_CH_1 1
#define DRV_ADS1220_D_CH_2 2
#define DRV_ADS1220_D_CH_3 3
#define DRV_ADS1220_D_CH_4 4
#define DRV_ADS1220_D_CH_5 5
#define DRV_ADS1220_D_CH_6 6
#define DRV_ADS1220_D_CH_7 7

void drv_adc_init(void);
float drv_adc_single_read_voltage(int channel, uint16_t avg, uint8_t *err);
float drv_adc_diff_read_voltage(int channel, uint16_t avg, uint8_t *err);
int32_t drv_adc_single_raw_read(int channel, uint16_t avg, uint8_t *err);
int32_t drv_adc_diff_raw_read(int channel, uint16_t avg, uint8_t *err);
void drv_adc_set_offset(int channel, float offset);
bool driver_adc_get(int channel, float *p_offset);

#endif