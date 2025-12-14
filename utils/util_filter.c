#include "util_filter.h"
#include <math.h>
#include "util_memory.h"

// float ±16,777,216
float recursive_avg(double pre_avg, float adc, int cnt)
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

float recursive_moving_avg(float prev_avg, int new_sample, int old_sample, int N)
{
  return prev_avg + ((float)new_sample - (float)old_sample) / (float)N;
}

//값이 최소값이하이며 절대오차 벗어나면 에러, 그외 최저값 사용
float validate_sensor_value_min(float value, float min, float abs_tol, uint8_t* err)
{
  if (err)
    *err = 0;



    if (value < min)
    {
      if (less_equal_float(fabsf(value - min), abs_tol))
      {
        return min; // 보정하여 반환
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

  if (isnan(value))
  {
    if(err)
    {
    *err = 1;
    }
    return NAN;
  }

  if (value > max)
  {
    if (less_equal_float(fabsf(value - max),abs_tol))
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