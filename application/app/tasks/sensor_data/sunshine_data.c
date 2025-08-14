
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

/**
 * @brief 일조 계산
 */
void calculate_sunshine(void)
{
  DATE_TIME_BUF ct = Date_Time;
  uint16_t *p_sunshine_1min=NULL;
  uint16_t *p_sunshine_days=NULL ;

  ct = Date_Time;

  p_sunshine_1min = user_malloc(SUNSHINE_TOTAL);
  if(p_sunshine_1min == NULL)
  {
    return;
  }

  p_sunshine_days = user_malloc(SUNSHINE_DAYS_SIZE);

  if(p_sunshine_days == NULL)
  {
    user_free(p_sunshine_1min);
    return;
  }

  memset(p_sunshine_1min, 0, SUNSHINE_TOTAL);
  memset(p_sunshine_days, 0, SUNSHINE_DAYS_SIZE);


  if (read_sunshine_1min(ct.Year, p_sunshine_1min, SUNSHINE_TOTAL) == 0)
  {
    compute_daily_data(DATA_SIZE_16, p_sunshine_1min, p_sunshine_days, ct.Year);

    g_sunshine.ten_min = get_10min_accu(DATA_SIZE_16, p_sunshine_1min, ct.Year, ct.Month, ct.Day, ct.Hour, ct.Min);
    g_sunshine.today   = get_daily_accu(DATA_SIZE_16, p_sunshine_days, ct.Year, ct.Month, ct.Day);
    g_sunshine.hourly  = get_hourly_accu(DATA_SIZE_16, p_sunshine_1min, ct.Year, ct.Month, ct.Day, ct.Hour, ct.Min);
    g_sunshine.monthly = get_monthly_accu(DATA_SIZE_16, p_sunshine_days, ct.Year, ct.Month);
    g_sunshine.yearly  = get_yearly_accu(DATA_SIZE_16, p_sunshine_days, ct.Year, ct.Month, ct.Day);

  }

  user_free(p_sunshine_1min);
  user_free(p_sunshine_days);

}