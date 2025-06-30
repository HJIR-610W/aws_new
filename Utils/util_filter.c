#include "util_filter.h"
#include <math.h>
// float ±16,777,216
float recursiveAvg(double pre_avg, float adc, int cnt)
{
  float avg;

  avg = ((cnt - 1) * pre_avg) / cnt + adc / cnt;

  return avg;
}

float recursive_avg_i(float pre_avg, int32_t adc, int cnt)
{
  float avg;

  avg = ((cnt - 1) * pre_avg) / cnt + (float)adc / (float)cnt;

  return avg;
}

//값이 최소값이하이며 절대오차 벗어나면 에러, 그외 최저값 사용
float validate_sensor_value_min(float value, float min, float abs_tol, uint8_t* err)
{
  if (err)
    *err = 0;

  if (value < min)
  {
    if (fabsf(value - min) <= abs_tol)
    {
      return min;  // 보정하여 반환
    }
    else
    {
      if (err)
        *err = 1;
      return value;
    }
  }

  return value;  // 정상값
}

float validate_sensor_value_max(float value, float max, float abs_tol, uint8_t* err)
{
  if (err)
    *err = 0;

  if (value > max)
  {
    if (fabsf(value - max) <= abs_tol)
    {
      return max;  // 보정하여 반환
    }
    else
    {
      if (err)
        *err = 1;
      return value;
    }
  }

  return value;  // 정상값
}