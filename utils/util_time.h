
#ifndef UTILE_TIME_H
#define UTILE_TIME_H


#include <time.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct 
{
  int16_t Year;
  int8_t Month;
  int8_t Day;
  int8_t Hour;
  int8_t Min;
  int8_t Sec;
  int8_t Week;
  uint16_t SubSec;
}DATE_TIME_BUF;


void time_cvt_secTotime(time_t sec,DATE_TIME_BUF *timeNow);
time_t time_cvt_timestamp(DATE_TIME_BUF *tN);
time_t time_set_time(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec);
time_t time_timestamp(void);

uint32_t time_get_year(time_t tmIn);
uint32_t time_get_month(time_t tmIn);
uint32_t time_get_day(time_t tmIn);
unsigned long time_get_seconds(time_t ts);

int is_leap_year(int year);
bool is_valid_datetime(const DATE_TIME_BUF *nt);
uint64_t cvt_timestamp_64(DATE_TIME_BUF *ct);
void subtract_seconds(DATE_TIME_BUF *dt, uint32_t seconds);
uint32_t offset_min(DATE_TIME_BUF *t);
uint32_t count_min(DATE_TIME_BUF *st, DATE_TIME_BUF *et);
uint32_t day_of_year(int year, int month, int day);

int32_t make_time_to_string(DATE_TIME_BUF *ct, char *out, uint16_t outSize);
extern DATE_TIME_BUF Date_Time;

#endif
