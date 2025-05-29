
#ifndef UTILE_DATA_H

#define UTILE_DATA_H

#include <stdint.h>

#include "utile_time.h"
#define DATA_SIZE_16 16
#define DATA_SIZE_32 32

void compute_daily_data(uint8_t type, void *data_minutes, void *data_days, int year);
void compute_monthly_data(uint8_t type, const void *data_days, int year, uint32_t *data_month);

int32_t get_daily_accu(uint8_t type, const void *data_days, int year, int month, int day);
uint16_t get_monthly_accu(uint8_t type, const void *data_days, int year, int month);

uint32_t get_yearly_accu(uint8_t type, const void *data_days, int year, int month, int day);
uint32_t get_hourly_accu(uint8_t type, const void *data_minutes, int year, int month, int day,
                         int hour, int min);

int get_minute_index(int year, int month, int day, int hour, int min);


uint32_t get_10min_accu(uint8_t type, const void *rain_minutes, int year, int month, int day,
  int hour, int min);

int write_bulk_data_range(const char *name, const char *start_datetime, const char *end_datetime,
                          uint16_t value);

    int read_bulk_data(const char *name, const DATE_TIME_BUF *start_time, uint32_t read_cnt,
                       uint16_t *buffer);
#endif