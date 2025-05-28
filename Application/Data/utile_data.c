#define __STDC_WANT_LIB_EXT1__ 1

#include "utile_data.h"

#include <math.h>

#include "app_file.h"
#include "cmsis_os2.h"
#include "dev_io.h"
#include "os_define.h"
#include "user_heap.h"
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


uint32_t get_10min_accu(uint8_t type, const void *rain_minutes, int year, int month, int day,
                        int hour, int min)
{
  if (!rain_minutes || hour < 0 || hour >= 24 || min < 0 || min >= 60)
    return 0xFFFFFFFF;

  uint32_t offset = get_minute_index(year, month, day, hour, min);
  uint32_t sum = 0;
  uint32_t read_cnt = (min % 10) + 1;

  if (type == 16)
  {
    const uint16_t *src = (const uint16_t *)rain_minutes;
    while (read_cnt--)
    {
      sum += src[offset--];
    }
  }
  else if (type == 32)
  {
    const uint32_t *src = (const uint32_t *)rain_minutes;
    while (read_cnt--)
    {
      sum += src[offset--];
    }
  }
  else
  {
    return 0xFFFFFFFF;  
  }

  return sum;
}

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ff.h"  // FatFs header

#define RECORD_SIZE 2
#define FIXED_FILE_MINUTES (366 * 24 * 60)
#define FIXED_FILE_SIZE ((FIXED_FILE_MINUTES + 1) * RECORD_SIZE)

// FatFs file object
static FIL file;

int is_leap_year(int year) { return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); }

int get_valid_minutes(int year) { return (is_leap_year(year) ? 366 : 365) * 24 * 60; }


void make_filename(int year, char *file_path,const char *filename)
{ 
  sprintf(file_path, "0:Y%02d/%s", year,filename); 
}

int parse_datetime(const char *datetime_str, struct tm *out)
{
  int y, M, d, h, m, s;
  if (sscanf(datetime_str, "%d-%d-%d %d:%d:%d", &y, &M, &d, &h, &m, &s) != 6)
    return 0;

  out->tm_year = y - 1900;
  out->tm_mon = M - 1;
  out->tm_mday = d;
  out->tm_hour = h;
  out->tm_min = m;
  out->tm_sec = s;
  return 1;
}

int calculate_offset(struct tm *t)
{
  struct tm base = {0};
  base.tm_year = t->tm_year;
  base.tm_mon = 0;
  base.tm_mday = 1;
  base.tm_hour = 0;
  base.tm_min = 1;
  base.tm_sec = 0;

  time_t t_base = mktime(&base);
  time_t t_now = mktime(t);
  return (int)((t_now - t_base) / 60) ;
}

int ensure_file_exists(const char *filename)
{
  FRESULT fr;
  FILINFO finfo;
  fr = f_stat(filename, &finfo);
  if (fr != FR_OK)
  {
    fr = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK)
      return -1;

    uint16_t zero = 0;
    for (uint32_t i = 0; i < FIXED_FILE_MINUTES + 1; ++i)
    {
      UINT bw;
      f_write(&file, &zero, RECORD_SIZE, &bw);
    }
    f_close(&file);
  }
  return 0;
}

int write_bulk_data_range(const char *name, const char *start_datetime,
                              const char *end_datetime, uint16_t value)
{
  struct tm start_tm = {0}, end_tm = {0};
  if (!parse_datetime(start_datetime, &start_tm) || !parse_datetime(end_datetime, &end_tm))
    return -1;

  time_t t_start = mktime(&start_tm);
  if (calculate_offset(&start_tm) <= 0)
  {
    return -99;  // offset 0인 경우 에러 리턴: 최소 1분부터 시작
  }
  time_t t_end = mktime(&end_tm);
  if (t_end < t_start)
    return -2;

  OS_SEM_PEND(get_file_sem(), -1);

  int start_year = start_tm.tm_year + 1900;
  int end_year = end_tm.tm_year + 1900;

  for (int year = start_year; year <= end_year; ++year)
  {
    char filename[64];
    make_filename(year % 10, filename, name);
    if (ensure_file_exists(filename) < 0)
    {
      OS_SEM_POST(get_file_sem());
      return -3;
    }

    int offset_start = 1;
    int offset_end = 0;

    if (year == start_year && year == end_year)
    {
      offset_start = calculate_offset(&start_tm);
      offset_end = calculate_offset(&end_tm);
      if (offset_end == 0)
        offset_end = get_valid_minutes(year - 1) + 1;
    }
    else if (year == start_year)
    {
      offset_start = calculate_offset(&start_tm);
      offset_end = get_valid_minutes(year);
    }
    else if (year == end_year)
    {
      offset_start = 1;
      offset_end = calculate_offset(&end_tm);
      if (offset_end == 0)
        offset_end = get_valid_minutes(year - 1) + 1;
    }
    else
    {
      offset_start = 1;
      offset_end = get_valid_minutes(year);
    }

    int count = offset_end - offset_start + 1;
    if (count <= 0)
      continue;

    uint16_t *buffer = (uint16_t *)aws_malloc(count * sizeof(uint16_t));
    if (!buffer)
    {
      OS_SEM_POST(get_file_sem());
      return -4;
    }

    FRESULT fr = f_open(&file, filename, FA_READ | FA_WRITE);
    if (fr != FR_OK)
    {
      aws_free(buffer);
      OS_SEM_POST(get_file_sem());
      return -5;
    }

    UINT br, bw;
    f_lseek(&file, offset_start * RECORD_SIZE);
    f_read(&file, buffer, count * RECORD_SIZE, &br);

    for (int i = 0; i < count; ++i) buffer[i] = value;

    f_lseek(&file, offset_start * RECORD_SIZE);
    f_write(&file, buffer, count * RECORD_SIZE, &bw);
    f_close(&file);
    aws_free(buffer);
  }

  OS_SEM_POST(get_file_sem());
  return 0;
}

int read_bulk_data(const char *name, const char *start_datetime, uint32_t read_cnt,
                   uint16_t *buffer)
{
  struct tm start_tm = {0};
  if (!parse_datetime(start_datetime, &start_tm))
    return -1;

  time_t current_time = mktime(&start_tm);
  if (calculate_offset(&start_tm) <= 0)
    return -2;

  OS_SEM_PEND(get_file_sem(), -1);

  int year = start_tm.tm_year + 1900;
  int offset = calculate_offset(&start_tm);
  int total_read = 0;

  while (read_cnt > 0)
  {
    char filename[64];
    make_filename(year % 10, filename, name);
    if (ensure_file_exists(filename) < 0)
    {
      OS_SEM_POST(get_file_sem());
      return -3;
    }

    int max_offset = get_valid_minutes(year);
    int remain_in_year = max_offset - offset + 1;

    int to_read = (read_cnt < (uint32_t)remain_in_year) ? read_cnt : remain_in_year;

    FRESULT fr = f_open(&file, filename, FA_READ);
    if (fr != FR_OK)
    {
      OS_SEM_POST(get_file_sem());
      return -4;
    }

    f_lseek(&file, offset * RECORD_SIZE);
    UINT br;
    f_read(&file, buffer + total_read, to_read * RECORD_SIZE, &br);
    f_close(&file);

    total_read += to_read;
    read_cnt -= to_read;

    year++;
    offset = 1;
  }

  OS_SEM_POST(get_file_sem());
  return total_read;
}
