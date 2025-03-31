
#ifndef HJ_TEMPERATURE_H
#define HJ_TEMPERATURE_H

#include "driver_interface.h"

#define HJ_TEMPERATURE 0

driver_t *hjTemperature_open(int32_t num, void *opt);
float hjTemperature_read(driver_t *driver, uint8_t *err);
#endif