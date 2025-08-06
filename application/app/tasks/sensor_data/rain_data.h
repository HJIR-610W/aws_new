
#ifndef RAIN_DATA_H
#define RAIN_DATA_H

#include "util_time.h"
#define RAIN_1MIN_FILE_NAME "RAIN_01.rcd"

int32_t rain_file_zero(int year) ;
void calculate_rain(void);
int32_t read_rain_1min(uint16_t year, uint16_t *rain_data, uint32_t read_size);

int get_hourly_rain(uint16_t *rain_minutes, int year, int month, int day, int hour,
                    int min);

uint16_t get_daily_rain(const uint16_t *rain_days, int year, int month, int day);
uint16_t get_monthly_rain(const uint16_t *rain_days, int year, int month, int day);
uint16_t get_yearly_rain(const uint16_t *rain_days, int year, int month, int day);
void compute_daily_rain(uint16_t *rain_minutes, uint16_t *rain_days, int year);

uint32_t get_10min_rain(const uint16_t *rain_minutes, int year, int month, int day, int hour,
                        int min);
#endif