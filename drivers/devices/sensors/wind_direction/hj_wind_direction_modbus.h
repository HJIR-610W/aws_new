
#ifndef HJ_WIND_DIR_MODBUS_HHH
#define HJ_WIND_DIR_MODBUS_HHH

#include <stdint.h>



int32_t hj_wind_direction_init(void *opt);
float hj_wind_direction_read(uint8_t *err);

#endif
