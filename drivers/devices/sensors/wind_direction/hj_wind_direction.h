
#ifndef HJ_WIND_DIRECTION_H

#define HJ_WIND_DIRECTION_H

#include "debug_io.h"
#include "Sensors\wind_speed\wind_define.h"
#include "sensors\wind_speed\hj_wind.h"
#define HJ_WIND_DIRECTION 101

#define HJ_WIND_CHANNEL_DIRECTION 2

driver_t *hjwind_direction_open(void *opt);

#endif