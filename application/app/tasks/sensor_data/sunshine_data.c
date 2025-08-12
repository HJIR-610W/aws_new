
#include "sunshine_data.h"
#include <stdio.h>
#include <string.h>
#include "user_heap.h"
#include "util_time.h"
#include "utile_data.h"

#include "rain_data.h"
#include "aws_data.h"
#include "ff.h"

#include "app_dataLogging.h"

#include "app_file.h"

// 1분마다 저장된 1년치 일조 데이터 읽기
int32_t read_sunshine_1min(uint16_t year, uint16_t *sunshine_data, uint32_t read_size)
{
  char path[50];
  FRESULT fret;
  // FSIZE_t file_size = 0;

  make_sunshine_1min_path(year, path, sizeof(path));

  fret = read_file(path, (uint8_t *)sunshine_data, read_size, 0);

  if (fret == FR_OK)
  {
    return 0;
  }

  return 1;
}

int32_t sunshine_file_zero(int year)
{
  char start_time[30];
  char end_time[30]; // 2025-01-01 00:00:00
  int32_t value = 0;
  int32_t ret;

  snprintf(start_time, sizeof(start_time), "%04d-01-01 00:01:00", year);
  snprintf(end_time, sizeof(end_time), "%04d-01-01 00:00:00", year + 1);


  ret = write_bulk_data_range(SUNSHINE_1MIN_FILE_NAME, start_time, end_time, value);

  return ret;
}



#define SUNSHINE_TOTAL (sizeof(uint16_t) * 60 * 24 * 366 + sizeof(uint16_t))
#define SUNSHINE_DAYS_SIZE (366 * sizeof(uint16_t))

// 일조
void calculate_sunshine(void)
{
  DATE_TIME_BUF ct = Date_Time;
  uint16_t daily_sunshine = 0;
  uint16_t hourly_sunshine = 0;
  uint16_t monthly_sunshine = 0;
  uint16_t yearly_sunshine = 0;
  uint16_t *p_sunshine_1min = user_malloc(SUNSHINE_TOTAL);
  uint16_t *p_sunshine_days = user_malloc(SUNSHINE_DAYS_SIZE);

  (void)daily_sunshine;
  (void)hourly_sunshine;
  ct = Date_Time;

  memset(p_sunshine_1min, 0, SUNSHINE_TOTAL);
  memset(p_sunshine_days, 0, SUNSHINE_DAYS_SIZE);
  if (read_sunshine_1min(ct.Year, p_sunshine_1min, SUNSHINE_TOTAL) == 0)
  {
    compute_daily_data(DATA_SIZE_16, p_sunshine_1min, p_sunshine_days, ct.Year);
    daily_sunshine = get_daily_accu(DATA_SIZE_16, p_sunshine_days, ct.Year, ct.Month, ct.Day);
    hourly_sunshine = get_hourly_accu(DATA_SIZE_16, p_sunshine_1min, ct.Year, ct.Month, ct.Day, ct.Hour, ct.Min);
    monthly_sunshine = get_monthly_accu(DATA_SIZE_16, p_sunshine_days, ct.Year, ct.Month);
    yearly_sunshine = get_yearly_accu(DATA_SIZE_16, p_sunshine_days, ct.Year, ct.Month, ct.Day);


    set_sunshine_today(daily_sunshine);
    set_sunshine_monthly(monthly_sunshine);
    set_sunshine_yearly(yearly_sunshine);

    user_free(p_sunshine_1min);
    user_free(p_sunshine_days);
  }

}