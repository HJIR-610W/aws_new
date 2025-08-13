

#ifndef WIND_DATA_H
#define WIND_DATA_H

#include <stdint.h>

typedef enum
{
  eWIND_MAX_1MIN,
  eWIND_MAX_10MIN,
  eWIND_MAX_DAY,
  eWIND_MAX
} eWIND_MAX_t;

void calculate_uv(double theta_deg, double s, double *u, double *v);
void calculate_wind(double u, double v, double *speed, double *direction_deg);

void add_wind_sample(float speed, float direction);
void calculate_wind_moving_avg(float *wind_speed, float *wind_direction);

void update_wind_vector_avg_1min(float speed, float direction);
void calculate_wind_avg_1min(float *speed, float *direction);

void calculate_wind_max(eWIND_MAX_t wind, int32_t speed, int32_t direction);
void read_wind_max(eWIND_MAX_t wind, int32_t *speed, int32_t *direction);
void wind_max_init(eWIND_MAX_t wind);

#endif