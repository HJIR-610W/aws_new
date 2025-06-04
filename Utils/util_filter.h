#ifndef UTILE_FILTER_H

#define UTILE_FILTER_H

#include <stdint.h>

// float ¡¾16,777,216
float recursiveAvg(double pre_avg, float adc, int cnt);

float recursive_avg_i(float pre_avg, int32_t adc, int cnt);
float validate_sensor_value_max(float value, float min, float abs_tol, uint8_t* err);
float validate_sensor_value_min(float value, float max, float abs_tol, uint8_t* err);
#endif