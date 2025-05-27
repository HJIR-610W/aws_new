

#include "data_accu.h"

#include <stdint.h>
#include <string.h>

#include "app_dataLogging.h"
#include "app_file.h"
#include "dev_io.h"
#include "ff.h"  // FatFs 관련 헤더
#include "task_logging.h"
#include "user_heap.h"
#include "utile_data.h"
#include "utile_time.h"

#define MINUTES_PER_DAY 1440
#define DAYS_IN_YEAR 366

// 1분마다 저장된 1년치 우량량 데이터 읽기
int32_t read_rain_1min(uint16_t year, uint16_t *rain_data, uint32_t read_size)
{
  char path[50];
  FRESULT fret;
  FSIZE_t file_size = 0;

  make_rain_1min_path(year, path, sizeof(path));

  fret = read_file(path, (uint8_t *)rain_data, read_size, 0);

  if (fret == FR_OK)
  {
    return 0;
  }

  return 1;
}

//1분마다 저장된 1년치 일조 데이터 읽기
int32_t read_sunshine_1min(uint16_t year, uint16_t *sunshine_data, uint32_t read_size)
{
  char path[50];
  FRESULT fret;
  FSIZE_t file_size = 0;

  make_sunshine_1min_path(year, path, sizeof(path));

  fret = read_file(path, (uint8_t *)sunshine_data, read_size, 0);

  if (fret == FR_OK)
  {
    return 0;
  }

  return 1;
}

//
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
  for (int i = 0; i <= min; i++)  // 0분부터 현재 분까지 포함 (min 포함)
  {
    sum += rain_minutes[start_index + i];
  }

  return (int)sum;
}


void compute_daily_rain(uint16_t *rain_minutes, uint16_t *rain_days, int year)
{
  uint32_t i;
  uint32_t day_index = 0;
  uint32_t minute_index = 1;  // 00:01부터 시작

  uint16_t days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (isLeapYear(year))
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

  if (isLeapYear(year))
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
    return -1;

  return rain_days[index - 1];  // 배열 index는 0-based
}

uint16_t get_monthly_rain(const uint16_t *rain_days, int year, int month, int day)
{
  uint16_t monthly_rain[12];

  compute_monthly_rain(rain_days, year, monthly_rain);

  return monthly_rain[month-1];
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
    sum += rain_days[i];  // rain_days는 0-based
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