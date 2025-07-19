

#ifndef BSP_ADC_H
#define BSP_ADC_H
#include <stdint.h>

#include <stdbool.h>

#define BSP_ADS1220_S_CH_0 0
#define BSP_ADS1220_S_CH_1 1
#define BSP_ADS1220_S_CH_2 2
#define BSP_ADS1220_S_CH_3 3
#define BSP_ADS1220_S_CH_4 4
#define BSP_ADS1220_S_CH_5 5
#define BSP_ADS1220_S_CH_6 6
#define BSP_ADS1220_S_CH_7 7
#define BSP_ADS1220_S_CH_8 8
#define BSP_ADS1220_S_CH_9 9
#define BSP_ADS1220_S_CH_10 10
#define BSP_ADS1220_S_CH_11 11
#define BSP_ADS1220_S_CH_12 12
#define BSP_ADS1220_S_CH_13 13
#define BSP_ADS1220_S_CH_14 14
#define BSP_ADS1220_S_CH_15 15
#define BSP_ADS1220_S_CH_16 16
#define BSP_ADS1220_S_CH_17 17
#define BSP_ADC_SYS_TEMP 18
#define BSP_ADC_SYS_BATTERY 19
#define BSP_ADS1220_D_CH_0 20
#define BSP_ADS1220_D_CH_1 21
#define BSP_ADS1220_D_CH_2 22
#define BSP_ADS1220_D_CH_3 23
#define BSP_ADS1220_D_CH_4 24
#define BSP_ADS1220_D_CH_5 25
#define BSP_ADS1220_D_CH_6 26
#define BSP_ADS1220_D_CH_7 27

#define BSP_ADC_MAX 28

void bsp_adc_init(void);
float bsp_adc_single_read_voltage(int channel, uint16_t avg, uint8_t *err);
float bsp_adc_diff_read_voltage(int channel, uint16_t avg, uint8_t *err);
void bsp_adc_set_offset(int channel, float offset);
bool bsp_adc_get_offset(int channel, float *p_offset);
int32_t bsp_adc_single_raw_read(int channel, uint16_t avg, uint8_t *err);
int32_t bsp_adc_diff_raw_read(int channel, uint16_t avg, uint8_t *err);

#endif