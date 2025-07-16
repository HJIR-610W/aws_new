#ifndef DRV_FREQUENCY_H
#define DRV_FREQUENCY_H

#include <stdint.h>
#include <stdbool.h>

#define DRV_FREQ_STM32_CH_0 0
#define DRV_FREQ_STM32_CH_1 1

void drv_frequency_init(void);
float drv_frequency_read(int channel, uint8_t *err);
void drv_frequency_start_measurement(int channel);
void drv_frequency_stop_measurement(int channel);
bool drv_frequency_is_measuring(int channel);

#endif