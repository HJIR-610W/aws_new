

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
void add_wind_vector_250ms(float wind_speed, float wind_direction);
void add_wind_vector_1min_samle(double u, double v);
void calculate_wind_vector_10s(double *p_u, double *p_v);
void calculate_wind_max(eWIND_MAX_t wind, float speed, float direction);
void read_wind_max(eWIND_MAX_t wind, float *speed, float *direction);
#endif