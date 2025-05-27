#define __STDC_WANT_LIB_EXT1__ 1

#include <math.h>
#include "utile_data.h"
#include "utile_time.h"

#define DATA_MINUTES_PER_DAY 1440

#define DATA_DAYS_IN_YEAR 366

void compute_daily_data(uint32_t *data_minutes, uint32_t *data_days, int year)
{
  uint32_t i;
  uint32_t day_index = 0;
  uint32_t minute_index = 1;  // 00:01부터 시작

  uint32_t days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (isLeapYear(year))
    days_in_month[1] = 29;

  for (int month = 0; month < 12; month++)
  {
    for (int day = 0; day < days_in_month[month]; day++)
    {
      uint32_t sum = 0;

      for (i = 0; i < DATA_MINUTES_PER_DAY; i++)
      {
        sum += data_minutes[minute_index++];
      }

      data_days[day_index++] = (uint16_t)sum;
    }
  }
}


void compute_monthly_data(const uint32_t *data_days, int year, uint32_t *data_month)
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
      sum += data_days[index++];
    }
    data_month[month] = (uint32_t)sum;
  }
}



uint32_t get_daily_accu(const uint32_t *data_days, int year, int month, int day)
{
  int index = dayOfYear(year, month, day);
  if (index <= 0 || index > 366)
    return -1;

  return data_days[index - 1];  // 배열 index는 0-based
}

uint16_t get_monthly_accu(const uint32_t *data_days, int year, int month, int day)
{
  uint32_t monthly_data[12];

  compute_monthly_data(data_days, year, monthly_data);

  return monthly_data[month - 1];
}

uint32_t get_yearly_accu(const uint32_t *data_days, int year, int month, int day)
{
  if (data_days == NULL)
    return 0;

  int doy = dayOfYear(year, month, day);
  if (doy <= 0 || doy > 366)
    return 0;

  uint16_t sum = 0;
  for (int i = 0; i < doy; i++)
  {
    sum += data_days[i];  //
  }

  return sum;
}

uint32_t get_hourly_accu(uint32_t *data_minutes, int year, int month, int day, int hour, int min)
{
  if (hour < 0 || hour >= 24 || min < 0 || min >= 60)
    return -1;

  int doy = dayOfYear(year, month, day);
  if (doy <= 0 || doy > DATA_DAYS_IN_YEAR)
    return -1;

  // 시작 시간은 항상 정시 0분
  uint32_t start_index = (doy - 1) * DATA_MINUTES_PER_DAY + hour * 60 + 0 + 1;

  uint32_t sum = 0;
  for (int i = 0; i <= min; i++)  // 0분부터 현재 분까지 포함 (min 포함)
  {
    sum += data_minutes[start_index + i];
  }

  return sum;
}

void index_to_datetime(uint16_t year, uint32_t index, DATE_TIME_BUF *dt)
{
  if (!dt)
    return;

  struct tm base_tm = {0};
  base_tm.tm_year = year - 1900;
  base_tm.tm_mon = 0;  
  base_tm.tm_mday = 1;
  base_tm.tm_hour = 0;
  base_tm.tm_min = 0;
  base_tm.tm_sec = 0;

  // index 분 만큼 추가
  time_t base_time = mktime(&base_tm);
  base_time += index * 60;

  struct tm result;

  localtime_s(&base_time, &result );

  // 결과 구조체 채우기
  dt->Year = result.tm_year + 1900;
  dt->Month = result.tm_mon + 1;
  dt->Day = result.tm_mday;
  dt->Hour = result.tm_hour;
  dt->Min = result.tm_min;
  dt->Sec = result.tm_sec;
  dt->Week = result.tm_wday;
  dt->SubSec = 0;
}

int get_minute_index(int year, int month, int day, int hour, int min)
{
  if (hour < 0 || hour > 23 || min < 0 || min > 59)
    return -1;

  int doy = dayOfYear(year, month, day);
  if (doy <= 0 || doy > DATA_DAYS_IN_YEAR)
    return -1;

  int index = (doy - 1) * DATA_MINUTES_PER_DAY + hour * 60 + min;
  return index;  // 0-based index
}
