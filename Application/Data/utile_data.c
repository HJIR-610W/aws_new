#define __STDC_WANT_LIB_EXT1__ 1

#include <math.h>
#include "utile_data.h"
#include "utile_time.h"

#define DATA_MINUTES_PER_DAY 1440

#define DATA_DAYS_IN_YEAR 366



void compute_daily_data(uint8_t type, void *data_minutes, void *data_days, int year)
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
        if (type == 16)
        {
          uint16_t *src = (uint16_t *)data_minutes;
          sum += src[minute_index++];
        }
        else if (type == 32)
        {
          uint32_t *src = (uint32_t *)data_minutes;
          sum += src[minute_index++];
        }
      }

      if (type == 16)
      {
        uint16_t *dst = (uint16_t *)data_days;
        dst[day_index++] = (uint16_t)sum;
      }
      else if (type == 32)
      {
        uint32_t *dst = (uint32_t *)data_days;
        dst[day_index++] = sum;
      }
    }
  }
}

void compute_monthly_data(uint8_t type, const void *data_days, int year, uint32_t *data_month)
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
      if (type == 16)
      {
        const uint16_t *src = (const uint16_t *)data_days;
        sum += src[index++];
      }
      else if (type == 32)
      {
        const uint32_t *src = (const uint32_t *)data_days;
        sum += src[index++];
      }
    }

      data_month[month] = sum;
 
  }
}

int32_t get_daily_accu(uint8_t type, const void *data_days, int year, int month, int day)
{
  int index = dayOfYear(year, month, day);
  if (index <= 0 || index > 366)
    return -1;  // 오류 값 (unsigned -1)

  index -= 1;  // 0-based 인덱스

  if (type == 16)
  {
    const uint16_t *src = (const uint16_t *)data_days;
    return (uint32_t)src[index];
  }
  else if (type == 32)
  {
    const uint32_t *src = (const uint32_t *)data_days;
    return src[index];
  }

  return -1;  // 잘못된 type
}

uint16_t get_monthly_accu(uint8_t type,const void *data_days, int year, int month)
{
  uint32_t monthly_data[12];

  compute_monthly_data(type,data_days, year, monthly_data);

  return monthly_data[month - 1];
}

uint32_t get_yearly_accu(uint8_t type, const void *data_days, int year, int month, int day)
{
  if (!data_days)
    return 0;

  int doy = dayOfYear(year, month, day);
  if (doy <= 0 || doy > 366)
    return 0;

  uint32_t sum = 0;

  if (type == 16)
  {
    const uint16_t *src = (const uint16_t *)data_days;
    for (int i = 0; i < doy; i++)
    {
      sum += src[i];
    }
  }
  else if (type == 32)
  {
    const uint32_t *src = (const uint32_t *)data_days;
    for (int i = 0; i < doy; i++)
    {
      sum += src[i];
    }
  }
  else
  {
    return 0;  // 잘못된 type
  }

  return sum;
}

uint32_t get_hourly_accu(uint8_t type, const void *data_minutes, int year, int month, int day,
                         int hour, int min)
{
  if (!data_minutes || hour < 0 || hour >= 24 || min < 0 || min >= 60)
    return 0xFFFFFFFF;

  int doy = dayOfYear(year, month, day);
  if (doy <= 0 || doy > DATA_DAYS_IN_YEAR)
    return 0xFFFFFFFF;

  // 시작 인덱스: 00분 기준 (주의: +1이면 00:01부터 시작하는 경우임, 여기선 00:00부터)
  uint32_t start_index = (doy - 1) * DATA_MINUTES_PER_DAY + hour * 60;

  uint32_t sum = 0;

  if (type == 16)
  {
    const uint16_t *src = (const uint16_t *)data_minutes;
    for (int i = 0; i <= min; i++)
    {
      sum += src[start_index + i];
    }
  }
  else if (type == 32)
  {
    const uint32_t *src = (const uint32_t *)data_minutes;
    for (int i = 0; i <= min; i++)
    {
      sum += src[start_index + i];
    }
  }
  else
  {
    return 0xFFFFFFFF;  // invalid type
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
