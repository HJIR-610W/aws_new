

#ifndef DRIVER_ADC_H
#define DRIVER_ADC_H

#include "driver_interface.h"
#include "driver_adc_define.h"

#define ADC_ADS1220 0
#define ADC_STM32   1

#define ADC_ADS1220_S_CH_0   0
#define ADC_ADS1220_S_CH_1   1 
#define ADC_ADS1220_S_CH_2   2
#define ADC_ADS1220_S_CH_3   3
#define ADC_ADS1220_S_CH_4   4
#define ADC_ADS1220_S_CH_5   5 
#define ADC_ADS1220_S_CH_6   6
#define ADC_ADS1220_S_CH_7   7
#define ADC_ADS1220_S_CH_8   8
#define ADC_ADS1220_S_CH_9   9
#define ADC_ADS1220_S_CH_10 10
#define ADC_ADS1220_S_CH_11 11
#define ADC_ADS1220_S_CH_12 12
#define ADC_ADS1220_S_CH_13 13
#define ADC_ADS1220_S_CH_14 14
#define ADC_ADS1220_S_CH_15 15

#define ADC_ADS1220_D_CH_0 0
#define ADC_ADS1220_D_CH_1 1
#define ADC_ADS1220_D_CH_2 2
#define ADC_ADS1220_D_CH_3 3
#define ADC_ADS1220_D_CH_4 4
#define ADC_ADS1220_D_CH_5 5
#define ADC_ADS1220_D_CH_6 6
#define ADC_ADS1220_D_CH_7 7


#define ADC_STM32_S_CH_0 0
#define ADC_STM32_S_CH_1 1


driver_t *driver_adc_open(uint32_t num,void *opt);
void driver_close(driver_t *handle);
int32_t driver_adc_single_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err);
int32_t driver_adc_diff_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err);
void driver_set(driver_t *handle, adc_set_option_t option, void *value);

#endif
