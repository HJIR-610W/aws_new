

#ifndef DRIVER_MUX_H
#define DRIVER_MUX_H

#include <stdint.h>
void adc_single_mux_set(uint16_t channel);
void adc_mux_init(void);
void adc_diff_mux_set(uint16_t channel);

#endif
