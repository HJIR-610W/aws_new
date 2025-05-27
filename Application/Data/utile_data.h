
#ifndef UTILE_DATA_H

#define UTILE_DATA_H

#include <stdint.h>


void compute_daily_data(uint32_t *data_minutes, uint32_t *data_days, int year);
void compute_monthly_data(const uint32_t *data_days, int year, uint32_t *data_month);
uint32_t get_daily_accu(const uint32_t *data_days, int year, int month, int day);
uint16_t get_monthly_accu(const uint32_t *data_days, int year, int month, int day);
uint32_t get_yearly_accu(const uint32_t *data_days, int year, int month, int day);
uint32_t get_hourly_accu(uint32_t *data_minutes, int year, int month, int day, int hour, int min);
int get_minute_index(int year, int month, int day, int hour, int min);

#endif