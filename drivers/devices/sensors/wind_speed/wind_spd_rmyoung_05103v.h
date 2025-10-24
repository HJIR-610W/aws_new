

#ifndef WIND_SPEED_RMYOUNG_05103V_H
#define WIND_SPEED_RMYOUNG_05103V_H

#include <stdint.h>
int32_t wind_spd_rmyoung_05103v_init(void *opt);
float read_wind_spd_rmyoung_05103v(uint8_t *err);
#endif