

#ifndef RAIN_PRESENT_H
#define RAIN_PRESENT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "app_sensor.h"
#include "driver_interface.h"

#define RAIN_PRESENT_DI 100
#define RAIN_PRESENT_ANALOG 101

driver_t *rainPresent_open(int32_t num,void *opt);
bool read_sensor_rainPresent(driver_t *driver,uint8_t *err);

#endif
