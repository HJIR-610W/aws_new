
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
#include "task_logging.h"
#define TEMP_BUFF_SIZE 16384
// 1분마다 저장된 1년치 일조 데이터 읽기
int32_t read_sunshine_1min(uint16_t year, uint16_t *p_buffer, uint32_t read_size)
{
  
    #if 0 
  char path[50];
  FRESULT fret;
  uint8_t *p_target = (uint8_t *)p_buffer;
  
  uint8_t *p_temp = user_malloc(TEMP_BUFF_SIZE);

  make_sunshine_1min_path(year, path, sizeof(path));

  int quot = read_size / TEMP_BUFF_SIZE;
  int rem = read_size % TEMP_BUFF_SIZE;

  for(int i = 0;i< quot; i++)
  {
    fret = read_file(path, (uint8_t *)p_temp, TEMP_BUFF_SIZE, i * TEMP_BUFF_SIZE);
    memcpy((uint8_t *)&p_target[i * TEMP_BUFF_SIZE], p_temp, TEMP_BUFF_SIZE);
  }
  
  if(rem)
  {
    fret = read_file(path, (uint8_t *)p_temp, rem, quot * TEMP_BUFF_SIZE);
    memcpy(&p_target[quot * TEMP_BUFF_SIZE], p_temp, rem);
  }

  user_free(p_temp);
  if(fret == FR_OK)
  {
    return 0;
  }
  
  return 1;
#else
  char path[50];
  FRESULT fret;

  make_sunshine_1min_path(year, path, sizeof(path));

  for(int i = 0 ; i< 2; i++)
  {
    fret = read_file(path, (uint8_t *)p_buffer, read_size, 0);

    if(fret == FR_OK)
    {
      return 0;
    }
    else
    {
      log_write(L_ERROR,"read sunshine file failed");
    }
  }
  return 1;
#endif
}

int32_t sunshine_file_zero(int year)
{
  DATE_TIME_BUF start_time;
  DATE_TIME_BUF end_time;
  int32_t value = 0;
  int32_t ret;

  start_time.Year = year;
  start_time.Month = 1;
  start_time.Day = 1;
  start_time.Hour =0;
  start_time.Min = 1;
  start_time.Sec = 0;

  end_time.Year = year+1;
  end_time.Month = 1;
  end_time.Day = 1;
  end_time.Hour = 0;
  end_time.Min = 1;
  end_time.Sec = 0;

  ret = write_bulk_data_range(SUNSHINE_1MIN_FILE_NAME, &start_time, &end_time, value);

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