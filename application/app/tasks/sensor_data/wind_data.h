

#ifndef WIND_DATA_H
#define WIND_DATA_H

#include <stdint.h>

void calculate_uv(double theta_deg, double s, double *u, double *v);
void calculate_wind(double u, double v, double *speed, double *direction_deg);

#endif