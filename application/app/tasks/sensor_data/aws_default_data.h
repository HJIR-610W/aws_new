

#ifndef AWS_DEFAULT_DATA_H
#define AWS_DEFAULT_DATA_H

#include <stdint.h>

typedef enum avg_1min_e
{
  eAVG_TEMPERATURE,
  eAVG_RELATIVE_HUMIDITY,
  eAVG_PRESSURE,
  eAVG_GROUND_TEMPERATURE,
  eAVG_SURFACE_TEMPERATURE,
  eAVG_SOIL_TEMPERATURE_5CM,
  eAVG_SOIL_TEMPERATURE_10CM,
  eAVG_SOIL_TEMPERATURE_20CM,
  eAVG_SOIL_TEMPERATURE_30CM,
  eAVG_SOIL_TEMPERATURE_50CM,
  eAVG_SOIL_TEMPERATURE_100CM,
  eAVG_SOIL_TEMPERATURE_150CM,
  eAVG_SOIL_TEMPERATURE_300CM,
  eAVG_SOIL_TEMPERATURE_500CM,
  eAVG_MAX
} eAVG_1MIN_TYPE_t;


typedef struct
{
  uint16_t count;
  float average;
} sensor_avg_t;

typedef struct
{
  int32_t max;
  int32_t min;
} sensor_min_max_t;

void update_sensor_avg(eAVG_1MIN_TYPE_t sensor, sensor_avg_t *p_sensor_avg, int32_t sample);
int32_t read_sensor_avg(eAVG_1MIN_TYPE_t sensor, sensor_avg_t *p_sensor_avg);
void sensor_avg_init(eAVG_1MIN_TYPE_t sensor, sensor_avg_t *p_sensor_avg);

void calculate_sensor_min_max(eAVG_1MIN_TYPE_t sensor, sensor_min_max_t *p_min_max, int32_t sample);
void read_sensor_min_max(eAVG_1MIN_TYPE_t sensor, sensor_min_max_t *p_min_max, int32_t *p_min, int32_t *p_max);
void sensor_min_max_init(eAVG_1MIN_TYPE_t sensor, sensor_min_max_t *p_min_max, int32_t min, int32_t max); 

extern sensor_avg_t g_sensor_avg_1min[eAVG_MAX];
extern sensor_avg_t g_sensor_avg_10min[eAVG_MAX];
extern sensor_avg_t g_sensor_avg_hour[eAVG_MAX];

extern sensor_min_max_t g_1min_min_max[eAVG_MAX];
#endif
