
#ifndef DRIVER_FREQINPUT_H
#define DRIVER_FREQINPUT_H

#include "driver_interface.h"

#define GENERAL_FREQ_1 0
#define GENERAL_FREQ_2 1

#define FREQ_MAX 2

driver_t *driver_freq_open(uint32_t num,const char *owner);
float driver_freq_read(driver_t *drv,uint8_t *err);
float driver_freq_read_duty(driver_t *drv,uint8_t *err);

uint16_t drv_freq_get_port_list(const char **list,uint16_t listMax);

extern const char *g_freq_owner_list[2];

#endif