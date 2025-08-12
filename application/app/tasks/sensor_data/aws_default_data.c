

#include "aws_default_data.h"

#include "app_sensor.h"

#define AVG_CNT 6


typedef struct 
{
  uint8_t count;
  uint8_t index;
  int32_t sample[AVG_CNT];
}avg_1min_t;

avg_1min_t avg_1min[eAVG_MAX];

void add_sample_1min(eAVG_1MIN_TYPE_t number, int32_t data)
{
  uint8_t sample_index;

  sample_index = avg_1min[number].index;

  avg_1min[number].sample[sample_index] = data;

  sample_index = (sample_index + 1) % AVG_CNT;

  avg_1min[number].index = sample_index;

  if (avg_1min[number].count < AVG_CNT)
  {
    avg_1min[number].count++;
  }
}

void calculate_1min_avg(eAVG_1MIN_TYPE_t number,int32_t *p_avg)
{
  int32_t sum = 0;
  uint8_t sample_count;
  uint8_t sample_index;
  uint8_t actual_count;

  sample_count = avg_1min[number].count;

  actual_count = (sample_count < AVG_CNT) ? sample_count : AVG_CNT;

  sample_index = avg_1min[number].index;

  uint8_t idx = (sample_index + AVG_CNT - 1) % AVG_CNT;
  for (uint8_t i = 0; i < actual_count; i++)
  {
    sum += avg_1min[number].sample[idx];

    idx = (idx + AVG_CNT - 1) % AVG_CNT;
  }

  *p_avg = sum / actual_count;
}
