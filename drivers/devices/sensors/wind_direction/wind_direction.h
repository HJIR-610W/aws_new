
#ifndef WIND_DIRECTION_H
#define WIND_DIRECTION_H

#include <stdint.h>

#include "app_sensor.h"
#include "app_adc.h"
#include "driver_interface.h"

#ifndef GENERAL_ADC
#define GENERAL_ADC 0
#endif
#ifndef GENERAL_RS485
#define GENERAL_RS485 1
#endif
#ifndef GENERAL_FREQ
#define GENERAL_FREQ 2
#endif

#define WIND_HJ_DIRECTION 101
#define WIND_DIRECTION_RMYOUNG_05103V 201
#define WIND_DIRECTION_HJ_MODBUS 301

driver_t *wind_direction_open(int32_t num, void *opt);
float wind_direction_read(driver_t *driver, int32_t channel, uint8_t *err);
#endif