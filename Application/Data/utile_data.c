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


int32_t last_minute_offsets_in_year(int32_t year)
{
  return get_minute_index(year,12,31,23,59)+1;
}


int read_bulk_data(const char *name, const DATE_TIME_BUF *start_time, uint32_t read_cnt,
                   uint16_t *buffer)
{
  FRESULT result = -1;
  DATE_TIME_BUF end_time;
  time_t start_sec = time_cvt_timestamp((DATE_TIME_BUF *)start_time);
  char file_path[64];
  uint32_t buffer_index = 0;
  uint32_t remain = read_cnt;

  memset(buffer, 0, sizeof(uint16_t) * read_cnt);

  if (offset_min((DATE_TIME_BUF *)start_time) == 0)
  {
    time_cvt_secTotime(start_sec - 60, &end_time);
    uint32_t offset = last_minute_offsets_in_year(end_time.Year);

    make_filename(end_time.Year % 10, file_path, name);
    result = read_file(file_path, (uint8_t*)&buffer[0], sizeof(uint16_t), offset * sizeof(uint16_t));
    if (result != FR_OK)
      return -1;

    if (read_cnt > 1)
    {
      DATE_TIME_BUF nt;
      time_cvt_secTotime(start_sec + 60, &nt);
      uint32_t offset = offset_min(&nt);

      make_filename(nt.Year % 10, file_path, name);
      result = read_file(file_path, (uint8_t*)&buffer[1], (read_cnt - 1) * sizeof(uint16_t),
                         offset * sizeof(uint16_t));
      if (result != FR_OK)
        return -1;
    }

    return (int)FR_OK;
  }

  time_cvt_secTotime(start_sec + (read_cnt * 60), &end_time);

  if (start_time->Year != end_time.Year)
  {
    uint32_t start_offset = offset_min((DATE_TIME_BUF *)start_time);
    uint32_t start_year_remain = last_minute_offsets_in_year(start_time->Year) - start_offset + 1;
    uint32_t to_read = (remain < start_year_remain) ? remain : start_year_remain;
    uint32_t read_bytes = to_read * sizeof(uint16_t);

    make_filename(start_time->Year % 10, file_path, name);
    result =
        read_file(file_path, (uint8_t*)&buffer[buffer_index], read_bytes, start_offset * sizeof(uint16_t));
    if (result != FR_OK)
      return -1;

    buffer_index += to_read;
    remain -= to_read;

    if (remain > 0)
    {
      uint32_t offset = 1;
      read_bytes = remain * sizeof(uint16_t);

      make_filename(end_time.Year % 10, file_path, name);
      result = read_file(file_path, (uint8_t*)&buffer[buffer_index], read_bytes, offset * sizeof(uint16_t));
      if (result != FR_OK)
        return -1;
    }
  }
  else
  {
    uint32_t offset = offset_min((DATE_TIME_BUF *)start_time);
    uint32_t read_bytes = remain * sizeof(uint16_t);

    make_filename(start_time->Year % 10, file_path, name);
    result = read_file(file_path, (uint8_t*)buffer, read_bytes, offset * sizeof(uint16_t));
    if (result != FR_OK)
      return -1;
  }

  return (int)FR_OK;
}


int parse_datetime_buf(const char *str, DATE_TIME_BUF *dt)
{
  int y, M, d, h, m, s;
  if (sscanf(str, "%04d-%02d-%02d %02d:%02d:%02d", &y, &M, &d, &h, &m, &s) != 6)
    return 0;
  dt->Year = (int16_t)y;
  dt->Month = (int8_t)M;
  dt->Day = (int8_t)d;
  dt->Hour = (int8_t)h;
  dt->Min = (int8_t)m;
  dt->Sec = (int8_t)s;
  return 1;
}


#if 0 
int write_bulk_data_range(const char *name, const char *start_datetime,
                          const char *end_datetime, uint16_t value)
{
  char path[50];
  DATE_TIME_BUF start_time, end_time;
  FRESULT fret;
  uint32_t write_cnt;

  if (!parse_datetime_buf(start_datetime, &start_time) ||
      !parse_datetime_buf(end_datetime, &end_time))
    {
      return -1;
    }
  
   write_cnt = count_min(&start_time, &end_time);

  if (offset_min(&start_time) == 0 && write_cnt > 0)
  {
    DATE_TIME_BUF yt;
    uint32_t offset;
    uint32_t start_sec = time_cvt_timestamp(&start_time);

    time_cvt_secTotime(start_sec - 60, &yt);

    offset = last_minute_offsets_in_year(yt.Year);

    make_filename(yt.Year % 10, path, name);
    fret = write_file(path, (uint8_t *)&value, sizeof(uint16_t), offset * sizeof(uint16_t));
    if (fret != FR_OK)
    {
      return -1;
    }

    write_cnt = count_min(&start_time,&end_time);
    write_cnt = write_cnt -1;
    if (write_cnt > 0)
    {
      DATE_TIME_BUF nt;
      uint32_t offset = 1;
      uint8_t *p_buffer = aws_malloc(write_cnt*sizeof(uint16_t));

      for(int i = 0;i<write_cnt;i++)
      {
        p_buffer[i] = value;
      }

      make_filename(start_time.Year % 10, path, name);
      fret = write_file(path, (uint8_t *)p_buffer, write_cnt * sizeof(uint16_t),
                        offset * sizeof(uint16_t));
      aws_free(p_buffer);
      if (fret != FR_OK)
      {
        return -1;
      }


    }

    return (int)FR_OK;
  }

  if(start_time.Year == end_time.Year)
  {
    uint32_t start_offset= offset_min(&start_time);
    uint32_t cnt = offset_min(&end_time) -  start_offset + 1;
    uint16_t *p_buffer = aws_malloc(cnt*sizeof(uint16_t));

    for(int i = 0 ; i< cnt; i++)
    {
      p_buffer[i] = value;
    }
    make_filename(start_time.Year % 10, path, name);

    fret = write_file(path, (uint8_t*)p_buffer, cnt * sizeof(uint16_t), start_offset);

    aws_free(p_buffer);
    
    if(fret !=FR_OK)
    {

      return -1;
    }
  }
  else
  {
    DATE_TIME_BUF last_time;
    uint32_t offset;
    uint32_t write_cnt;
    uint32_t total_cnt;
    uint32_t remain_cnt;
    total_cnt = count_min(&start_time,&end_time);
    offset = offset_min(&start_time);
    write_cnt = last_minute_offsets_in_year(start_time.Year)-offset +1;

    make_filename(start_time.Year % 10, path, name);

    uint16_t *p_buffer = aws_malloc(write_cnt*sizeof(uint16_t));

        if(p_buffer ==0)
    {
      return -1;
    }
    
    for(int i = 0;i< write_cnt; i++)
    {
      p_buffer[i] = value;
    }

    fret = write_file(path,(uint8_t *)p_buffer,write_cnt*sizeof(uint16_t),offset);
    aws_free(p_buffer);
    if(fret != FR_OK)
    {
      return -1;
    }

    remain_cnt = total_cnt - write_cnt;

    if (remain_cnt)
    {
      p_buffer = aws_malloc(remain_cnt * sizeof(uint16_t));
    
    if(p_buffer ==0)
    {
      return -1;
    }
      
      for (int i = 0; i < remain_cnt; i++)
    {
      p_buffer[i] = value;
    }
    make_filename(end_time.Year % 10, path, name);
    fret = write_file(path, (uint8_t *)p_buffer, remain_cnt * sizeof(uint16_t), sizeof(uint16_t));
    aws_free(p_buffer);
    if (fret != FR_OK)
    {
      return -1;
    }
    }
  }

  return 0;
}

#endif


#define VALUE_SIZE sizeof(uint16_t)

static int write_data_block(const char *name, uint16_t year, uint32_t offset, uint32_t count,
                            uint16_t value)
{
  if (count == 0)
    return 0;

  uint16_t *buffer = aws_malloc(count * VALUE_SIZE);
  if (!buffer)
    return -1;

  for (uint32_t i = 0; i < count; i++) buffer[i] = value;

  char path[50];
  make_filename(year % 10, path, name);

  int res = write_file(path, (uint8_t *)buffer, count * VALUE_SIZE, offset * VALUE_SIZE);
  aws_free(buffer);
  return (res == FR_OK) ? 0 : -1;
}

int write_bulk_data_range(const char *name, const char *start_datetime, const char *end_datetime,
                          uint16_t value)
{
  DATE_TIME_BUF start, end;
  if (!parse_datetime_buf(start_datetime, &start) || !parse_datetime_buf(end_datetime, &end))
    return -1;

  uint32_t total_count = count_min(&start, &end);
  if (total_count == 0)
    return 0;

  uint32_t start_offset = offset_min(&start);


  if (start_offset == 0)
  {
    DATE_TIME_BUF prev_min;
    time_cvt_secTotime(time_cvt_timestamp(&start) - 60, &prev_min);
    uint32_t last_offset = last_minute_offsets_in_year(prev_min.Year);
    if (write_data_block(name, prev_min.Year, last_offset, 1, value) != 0)
      return -1;


    start_offset = 1;
    total_count--;
    if (total_count == 0)
      return 0;
  }

  if (start.Year == end.Year)
  {
    return write_data_block(name, start.Year, start_offset, total_count, value);
  }
  else
  {
    uint32_t end_of_year_offset = last_minute_offsets_in_year(start.Year);
    uint32_t first_year_count = end_of_year_offset - start_offset + 1;
    if (write_data_block(name, start.Year, start_offset, first_year_count, value) != 0)
      return -1;

    uint32_t remaining = total_count - first_year_count;
    return write_data_block(name, end.Year, 1, remaining, value);
  }
}

#if 0 
int read_bulk_data(const char *name, const DATE_TIME_BUF *start_time, uint32_t read_cnt,
                   uint16_t *buffer)
{
  if (read_cnt == 0 || buffer == NULL || start_time == NULL)
    return -1;

  memset(buffer, 0, read_cnt * sizeof(uint16_t));

  time_t start_sec = time_cvt_timestamp((DATE_TIME_BUF *)start_time);
  DATE_TIME_BUF end_time;
  time_cvt_secTotime(start_sec + read_cnt * 60, &end_time);

  char file_path[64];
  uint32_t buffer_index = 0;
  FRESULT res;

  // Handle special case: offset == 0 (next year's 00:00:00)
  if (offset_min((DATE_TIME_BUF *)start_time) == 0)
  {
    DATE_TIME_BUF prev_min;
    time_cvt_secTotime(start_sec - 60, &prev_min);
    uint32_t offset = last_minute_offsets_in_year(prev_min.Year);

    make_filename(prev_min.Year % 10, file_path, name);
    res = read_file(file_path, (uint8_t *)&buffer[0], sizeof(uint16_t), offset * sizeof(uint16_t));
    if (res != FR_OK)
      return -1;

    // If only 1 minute requested
    if (read_cnt == 1)
      return FR_OK;

    // Prepare to read rest from offset 1 of next year
    DATE_TIME_BUF nt;
    time_cvt_secTotime(start_sec + 60, &nt);
    make_filename(nt.Year % 10, file_path, name);

    res = read_file(file_path, (uint8_t *)&buffer[1], (read_cnt - 1) * sizeof(uint16_t),
                    1 * sizeof(uint16_t));
    return (res == FR_OK) ? FR_OK : -1;
  }

  // Normal range handling
  if (start_time->Year == end_time.Year)
  {
    uint32_t offset = offset_min((DATE_TIME_BUF *)start_time);
    make_filename(start_time->Year % 10, file_path, name);
    res = read_file(file_path, (uint8_t *)buffer, read_cnt * sizeof(uint16_t),
                    offset * sizeof(uint16_t));
    return (res == FR_OK) ? FR_OK : -1;
  }
  else
  {
    // Cross-year case
    uint32_t start_offset = offset_min((DATE_TIME_BUF *)start_time);
    uint32_t this_year_max = last_minute_offsets_in_year(start_time->Year);
    uint32_t this_year_count = this_year_max - start_offset + 1;
    uint32_t remaining = read_cnt;

    uint32_t read_cnt_this_year = (remaining < this_year_count) ? remaining : this_year_count;

    make_filename(start_time->Year % 10, file_path, name);
    res = read_file(file_path, (uint8_t *)&buffer[buffer_index],
                    read_cnt_this_year * sizeof(uint16_t), start_offset * sizeof(uint16_t));
    if (res != FR_OK)
      return -1;

    buffer_index += read_cnt_this_year;
    remaining -= read_cnt_this_year;

    if (remaining > 0)
    {
      make_filename(end_time.Year % 10, file_path, name);
      res = read_file(file_path, (uint8_t *)&buffer[buffer_index], remaining * sizeof(uint16_t),
                      1 * sizeof(uint16_t));
      if (res != FR_OK)
        return -1;
    }

    return FR_OK;
  }
}
#endif