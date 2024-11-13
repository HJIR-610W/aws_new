
#ifndef DRIVER_FREQINPUT_H
#define DRIVER_FREQINPUT_H

#include "driver_interface.h"

#define FREQ_MEAURE_A 0
#define FREQ_MEAURE_B 1
#define FREQ_MEAURE_C 2

driver_t *driver_freq_open(uint32_t num);

void driver_freq_read(driver_t *drv,float *freq);
#endif