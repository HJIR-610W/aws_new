
#ifndef DRIVER_FREQINPUT_H
#define DRIVER_FREQINPUT_H

#include "driver_interface.h"

#define FREQ_MEAURE_B 0
#define FREQ_MEAURE_C 1

#define FREQ_MAX 2
driver_t *driver_freq_open(uint32_t num);

float driver_freq_read(driver_t *drv);
#endif