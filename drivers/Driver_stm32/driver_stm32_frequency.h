#ifndef DRIVER_STM32_FREQUENCY_H
#define DRIVER_STM32_FREQUENCY_H

#include <stdint.h>
#include <stdbool.h>

#define STM32_FREQ_CH_0 0
#define STM32_FREQ_CH_1 1
#define STM32_FREQ_MAX 2  // PCB 버전에 따라 TIM10/TIM11 또는 TIM2/TIM5 사용

void stm32_frequency_init(void);
float stm32_frequency_read(int channel, uint8_t *err);
void stm32_frequency_start_measurement(int channel);
void stm32_frequency_stop_measurement(int channel);
bool stm32_frequency_is_measuring(int channel);

#endif