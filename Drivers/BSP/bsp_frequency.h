#ifndef BSP_FREQUENCY_H
#define BSP_FREQUENCY_H

#include <stdint.h>
#include <stdbool.h>

#define BSP_FREQ_STM32_CH_0 0
#define BSP_FREQ_STM32_CH_1 1
#define BSP_FREQ_EXT_CH_0 2
#define BSP_FREQ_EXT_CH_1 3

#define BSP_FREQ_MAX 4

void bsp_frequency_init(void);
float bsp_frequency_read(int channel, uint8_t *err);
void bsp_frequency_start_measurement(int channel);
void bsp_frequency_stop_measurement(int channel);
bool bsp_frequency_is_measuring(int channel);

#endif