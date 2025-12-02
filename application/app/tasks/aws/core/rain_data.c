

#include <stdio.h>
#include <string.h>
#include "util_time.h"
#include "utile_data.h"
#include "user_heap.h"
#include "rain_data.h"
#include "aws_data.h"
#include "app_dataLogging.h"
#include "app_file.h"
#include "dev_io.h"
#include "ff.h" // FatFs 관련 헤더
#include "task_logging.h"





#define MINUTES_PER_DAY 1440
#define DAYS_IN_YEAR 366

#define TEMP_BUFF_SIZE 16384
// 1분마다 저장된 1년치 우량량 데이터 읽기
int32_t read_rain_1min(uint16_t year, uint16_t *p_buffer, uint32_t read_size)
{
  #if 0 
  char path[50];
  FRESULT fret;
  uint8_t *p_target = (uint8_t *)p_buffer;
  
  uint8_t *p_temp = user_malloc(TEMP_BUFF_SIZE);

  make_rain_1min_path(year, path, sizeof(path));

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
#else
  char path[50];
  FRESULT fret=FR_OK;
  int ret =1;
  make_rain_1min_path(year, path, sizeof(path));

  //2회 시도한다.
  for(int i = 0 ; i< 2; i++)
  {
    fret = read_file(path, (uint8_t *)p_buffer, read_size, 0);

    if(fret == FR_OK)
    {
      return 0;
    }
    else
    {
      log_printf(L_ERROR,"read rain file failed");
    }
  }
#endif

  return 1;
}


int get_hourly_rain(uint16_t *rain_minutes, int year, int month, int day, int hour,
                    int min)
{
  if (hour < 0 || hour >= 24 || min < 0 || min >= 60)
    return -1;

  int doy = dayOfYear(year, month, day);
  if (doy <= 0 || doy > DAYS_IN_YEAR)
    return -1;

  // 시작 시간은 항상 정시 0분DA
  uint32_t start_index = (doy - 1) * MINUTES_PER_DAY + hour * 60 + 0 + 1;

  uint32_t sum = 0;
  for (int i = 0; i <= min; i++) // 0분부터 현재 분까지 포함 (min 포함)
  {
    sum += rain_minutes[start_index + i];
  }

  return (int)sum;
}

void compute_daily_rain(uint16_t *rain_minutes, uint16_t *rain_days, int year)
{
  uint32_t i;
  uint32_t day_index = 0;
  uint32_t minute_index = 1; // 00:01부터 시작

  uint16_t days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (is_leap_year(year))
    days_in_month[1] = 29;

  for (int month = 0; month < 12; month++)
  {
    for (int day = 0; day < days_in_month[month]; day++)
    {
      uint32_t sum = 0;

      for (i = 0; i < MINUTES_PER_DAY; i++)
      {
        sum += rain_minutes[minute_index++];
      }

      rain_days[day_index++] = (uint16_t)sum;
    }
  }
}
void compute_monthly_rain(const uint16_t *rain_days, int year, uint16_t *rain_month)
{
  uint16_t days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (is_leap_year(year))
    days_in_month[1] = 29;

  uint32_t index = 0;

  for (int month = 0; month < 12; month++)
  {
    uint32_t sum = 0;
    for (int d = 0; d < days_in_month[month]; d++)
    {
      sum += rain_days[index++];
    }
    rain_month[month] = (uint16_t)sum;
  }
}

uint16_t get_daily_rain(const uint16_t *rain_days, int year, int month, int day)
{

  int index = dayOfYear(year, month, day);
  if (index <= 0 || index > 366)
    return 0;

  return rain_days[index - 1]; // 배열 index는 0-based
}

uint16_t get_monthly_rain(const uint16_t *rain_days, int year, int month, int day)
{
  uint16_t monthly_rain[12];

  compute_monthly_rain(rain_days, year, monthly_rain);

  return monthly_rain[month - 1];
}

/**
 * @brief 일간 우량 자료로 년간 우량 계산
 */
uint16_t get_yearly_rain(const uint16_t *rain_days, int year, int month, int day)
{
  if (rain_days == NULL)
    return 0;

  int doy = dayOfYear(year, month, day);
  if (doy <= 0 || doy > 366)
    return 0;

  uint16_t sum = 0;
  for (int i = 0; i < doy; i++)
  {
    sum += rain_days[i]; // rain_days는 0-based
  }

  return sum;
}

/**
 * @brief 1분 자료로 10분 우량 산출
 * 00:01:00 ~ 00:10:00 10분 누적 자료
 * 예)
 * 요청시간이 00:02:00이면
 * 00:01:00
 * 00:02:00
 * 누적 산출
 * 요청시간이 00:10:00이면
 * 00:10:00
 * 누적산출
 */
uint32_t get_10min_rain(const uint16_t *rain_minutes, int year, int month, int day, int hour,
                        int min)
{
  uint32_t offset = 0;
  uint32_t sum = 0;
  uint32_t read_cnt;

  offset = get_minute_index(year, month, day, hour, min);

  read_cnt = min % 10 + 1;
  while (read_cnt)
  {
    sum += rain_minutes[offset--];
    read_cnt--;
  }

  return sum;
}
int32_t rain_file_zero(int year)
{
  DATE_TIME_BUF start_time;
  DATE_TIME_BUF end_time;
  int32_t value = 0;
  int32_t ret;

  start_time.Year = year;
  start_time.Month = 1;
  start_time.Day = 1;
  start_time.Hour = 0;
  start_time.Min = 1;
  start_time.Sec = 0;

  end_time.Year = year + 1;
  end_time.Month = 1;
  end_time.Day = 1;
  end_time.Hour = 0;
  end_time.Min = 1;

  ret = write_bulk_data_range(RAIN_1MIN_FILE_NAME, &start_time, &end_time, value);

  return ret;
}

#define RAIN_TOTAL (sizeof(uint16_t) * 60 * 24 * 366 + sizeof(uint16_t))
#define RAIN_DAYS_SIZE (366 * sizeof(uint16_t))

/*
SD카드에 기록된 RAIN_01.rcd 1분 우량 파일을 전부 읽어서 연산
*/
void calculate_rain(void)
{
  DATE_TIME_BUF ct = Date_Time;
  uint16_t *p_rain_1min=NULL;
  uint16_t *p_rain_days =NULL;
  DATE_TIME_BUF pre_date;

  ct = Date_Time;
  pre_date = Date_Time;

  p_rain_1min = user_malloc(RAIN_TOTAL);
  if(p_rain_1min == NULL)
  return;

  p_rain_days = user_malloc(RAIN_DAYS_SIZE);
  if (p_rain_days == NULL)
  {
    user_free(p_rain_1min);
    return;
  }


  memset(p_rain_1min, 0, RAIN_TOTAL);
  memset(p_rain_days, 0, RAIN_DAYS_SIZE);
  if (read_rain_1min(ct.Year, p_rain_1min, RAIN_TOTAL) == 0)
  {
    compute_daily_rain(p_rain_1min, p_rain_days, ct.Year);

    g_rainfall.today = get_daily_accu(DATA_SIZE_16, p_rain_days, ct.Year, ct.Month, ct.Day);
    g_rainfall.hourly = get_hourly_accu(DATA_SIZE_16, p_rain_1min, ct.Year, ct.Month, ct.Day, ct.Hour, ct.Min);
    g_rainfall.monthly = get_monthly_accu(DATA_SIZE_16, p_rain_days, ct.Year, ct.Month);
    g_rainfall.yearly = get_yearly_accu(DATA_SIZE_16, p_rain_days, ct.Year, ct.Month, ct.Day);
    g_rainfall.ten_min = get_10min_accu(DATA_SIZE_16, p_rain_1min, ct.Year, ct.Month, ct.Day, ct.Hour, ct.Min);

    // 전일 우량 1일전 시간계산
    subtract_seconds(&pre_date, 86400);

    if (pre_date.Year != ct.Year)
    {
      if (read_rain_1min(pre_date.Year, p_rain_1min, RAIN_TOTAL) == 0)
      {
        compute_daily_rain(p_rain_1min, p_rain_days, pre_date.Year);
        g_rainfall.yesterday = get_daily_accu(DATA_SIZE_16, p_rain_days, pre_date.Year, pre_date.Month, pre_date.Day);
      }
    }
    else
    {
      g_rainfall.yesterday = get_daily_accu(DATA_SIZE_16, p_rain_days, pre_date.Year, pre_date.Month, pre_date.Day);
    }



  }

  user_free(p_rain_1min);
  user_free(p_rain_days);
}