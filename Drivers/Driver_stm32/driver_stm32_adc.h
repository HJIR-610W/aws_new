
#ifndef DRIVER_STM32_ADC_H
#define DRIVER_STM32_ADC_H

#include "driver_adc_define.h"

#define STM32_ADC_SE_CH_0 0
#define STM32_ADC_SE_CH_1 1
#define STM32_ADC_SE_MAX 2

void stm32_adc_init(void);
int32_t stm32_adc_read_single(int channel, uint16_t avgCnt, uint8_t *err);

#endif
