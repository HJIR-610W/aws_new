

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
} eAVG_DATA_TYPE_t;


typedef struct
{
  uint16_t count;
  float average;
} data_avg_t;

typedef struct
{
  int32_t max;
  int32_t min;
} data_min_max_t;

int32_t read_data_average(eAVG_DATA_TYPE_t type, data_avg_t *p_avg_buffer);
int32_t calculate_data_avg(eAVG_DATA_TYPE_t type, data_avg_t *p_avg_buffer, int32_t sample);
int32_t read_current_data_average(eAVG_DATA_TYPE_t type, data_avg_t *p_avg_buffer);
void data_avg_init(eAVG_DATA_TYPE_t type, data_avg_t *p_avg_buffer);


void calculate_data_min_max(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max, int32_t sample);

void data_min_max_init(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max, int32_t min, int32_t max); 


int32_t read_data_min(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max_buffer,int32_t set_min);
int32_t read_data_max(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max_buffer,int32_t set_max);

void aws_min_max_init(void);

int32_t read_current_data_min(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max);
int32_t read_current_data_max(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max) ;

extern data_avg_t g_avg_1min[eAVG_MAX];
extern data_avg_t g_avg_10min[eAVG_MAX];
extern data_avg_t g_avg_hour[eAVG_MAX];

extern data_min_max_t g_1min_min_max[eAVG_MAX];
extern data_min_max_t g_10min_min_max[eAVG_MAX];
extern data_min_max_t g_hour_min_max[eAVG_MAX];
extern data_min_max_t g_day_min_max[eAVG_MAX];
#endif
