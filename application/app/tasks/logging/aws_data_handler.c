
#include "util_time.h"
#include "utile_data.h"

#include <stdio.h>
int32_t rain_file_zero(int year)
{
  char start_time[30];
  char end_time[30]; // 2025-01-01 00:00:00
  int32_t value=0;
  const char *filename;
  int32_t ret;


  snprintf(start_time, sizeof(start_time), "%04d-01-01 00:01:00", year);
  snprintf(end_time, sizeof(end_time), "%04d-01-01 00:00:00", year + 1);

  filename = "RAIN_01.rcd";
  ret = write_bulk_data_range(filename, start_time, end_time, value);

  return ret;

}

int32_t sunshine_file_zero(int year)
{
  char start_time[30];
  char end_time[30]; // 2025-01-01 00:00:00
  int32_t value=0;
  const char *filename;
  int32_t ret;

  snprintf(start_time, sizeof(start_time), "%04d-01-01 00:01:00", year);
  snprintf(end_time, sizeof(end_time), "%04d-01-01 00:00:00", year + 1);

  filename = "SUNSHINE_01.rcd";
  ret = write_bulk_data_range(filename, start_time, end_time, value);

  return ret;
}

