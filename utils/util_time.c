#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>
#include <stdio.h>

#include "cmsis_os.h"
#include "util_time.h"

DATE_TIME_BUF Date_Time;





void time_cvt_secTotime(time_t sec,DATE_TIME_BUF *timeNow)
{
	struct tm newtime;

	localtime_s(&sec,&newtime);

	timeNow->Year = newtime.tm_year + 1900;
	timeNow->Month = newtime.tm_mon +1;
	timeNow->Day  =  newtime.tm_mday;
	timeNow->Hour = newtime.tm_hour;
	timeNow->Min = newtime.tm_min;
	timeNow->Sec = newtime.tm_sec;

}

time_t time_cvt_timestamp(DATE_TIME_BUF *tN)
{
	struct tm atm;

	atm.tm_sec = tN->Sec;
	atm.tm_min = tN->Min;
	atm.tm_hour = tN->Hour;
	atm.tm_mday = tN->Day;
	atm.tm_mon = tN->Month-1;       // tm_mon is 0 based
	atm.tm_year = tN->Year - 1900;     // tm_year is 1900 based
	atm.tm_isdst = 0;
    

	return mktime(&atm);
}

time_t time_timestamp(void)
{
	return time_cvt_timestamp(&Date_Time);
}

int32_t make_time_to_string(DATE_TIME_BUF *ct, char *out, uint16_t outSize)
{
  return snprintf_s(out,outSize,"%04d-%02d-%02d %02d:%02d:%02d",ct->Year,
  ct->Month,ct->Day,ct->Hour,ct->Min,ct->Sec);
}


time_t SetTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec)
{
	struct tm atm;

	atm.tm_sec = nSec;
	atm.tm_min = nMin;
	atm.tm_hour = nHour;
	atm.tm_mday = nDay;
	atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = 0;
	return mktime(&atm);
}

int GetYear(time_t tmIn)
{
    struct tm time_info;
  //C11
    time_info.tm_year =0;
	localtime_s(&tmIn,&time_info);
    
    return time_info.tm_year+1900;
}
int GetMonth(time_t tmIn)
{
    struct tm time_info;
  //C11
    time_info.tm_year =0;
    time_info.tm_mon = 0;
	localtime_s(&tmIn,&time_info);
    
    return time_info.tm_mon+1;
}

int GetDay(time_t tmIn)
{
  struct tm time_info;

  time_info.tm_year = 0;
  time_info.tm_mon = 0;
  localtime_s(&tmIn, &time_info);

  return time_info.tm_mday;
}

long GetTotalSeconds(time_t ts)
{
	return ts;
}


// year: 연도 (예: 2025)
// month: 월 (1 ~ 12)
// day: 일 (1 ~ 31)
// 리턴값: 해당 연도의 1월 1일부터 몇 번째 날인지 (1 ~ 365 또는 366)
int dayOfYear(int year, int month, int day)
{
  static const uint16_t days_until_month[12] = {0,   31,  59,  90,  120, 151,
                                                181, 212, 243, 273, 304, 334};
  static const uint16_t days_until_month_leap[12] = {0,   31,  60,  91,  121, 152,
                                                     182, 213, 244, 274, 305, 335};

  if (month < 1 || month > 12 || day < 1 || day > 31)
    return -1;  // 잘못된 날짜 입력

  const uint16_t *table = is_leap_year(year) ? days_until_month_leap : days_until_month;
  return table[month - 1] + day;
}

void subtract_seconds(DATE_TIME_BUF *dt, uint32_t seconds)
{
  uint32_t tick = time_cvt_timestamp(dt);

  tick = tick -  seconds;

  time_cvt_secTotime(tick,dt);

}


//현재 분이 해의 시작부터 몇번째 분인지 확인
int offset_min(DATE_TIME_BUF *t)
{
  DATE_TIME_BUF base;

  base.Year = t->Year;
  base.Month = 1;
  base.Day = 1;
  base.Hour = 0;
  base.Min = 0;
  base.Sec = 0;

  time_t t_base = time_cvt_timestamp(&base);
  time_t t_now = time_cvt_timestamp(t);

  int offset = (int)((t_now - t_base) / 60);

  return offset;
}

int32_t count_min(DATE_TIME_BUF *st,DATE_TIME_BUF *et)
{
  time_t t_et = time_cvt_timestamp(et);
  time_t t_st = time_cvt_timestamp(st);

  int offset = (int)((t_et - t_st) / 60)+1;

  return offset;
}

int is_leap_year(int year)
{ 
  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); 
}

// 각 월의 일수 (평년 기준)
static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

uint64_t cvt_timestamp_64(DATE_TIME_BUF* ct)
{
  uint64_t total_days = 0;

  // 1970년부터 해당 년도 전년까지의 일수 계산
  for (int y = 1970; y < ct->Year; y++)
  {
    total_days += is_leap_year(y) ? 366 : 365;
  }

  // 해당 년도에서 해당 월 전월까지의 일수 계산
  for (int m = 1; m < ct->Month; m++)
  {
    total_days += days_in_month[m - 1];
    // 2월이고 윤년인 경우 하루 추가
    if (m == 2 && is_leap_year(ct->Year))
    {
      total_days++;
    }
  }

  // 해당 월에서의 일수 추가 (day - 1: 1일은 0일째)
  total_days += (ct->Day - 1);

  // 총 초 계산
  uint64_t total_seconds = total_days * 24 * 60 * 60;
  total_seconds += ct->Hour * 60 * 60;
  total_seconds += ct->Min * 60;
  total_seconds += ct->Sec;

  return total_seconds;
}

// 64비트 timestamp를 다시 날짜로 변환하는 함수
void cvt_timestamp64_to_date(uint64_t timestamp, int *year, int *month, int *day,
                       int *hour, int *min, int *sec)
{
  uint64_t total_seconds = timestamp;

  // 시, 분, 초 계산
  *sec = total_seconds % 60;
  total_seconds /= 60;
  *min = total_seconds % 60;
  total_seconds /= 60;
  *hour = total_seconds % 24;
  total_seconds /= 24;

  // 총 일수
  uint64_t total_days = total_seconds;

  // 년도 계산
  *year = 1970;
  while (total_days >= (is_leap_year(*year) ? 366 : 365))
  {
    total_days -= is_leap_year(*year) ? 366 : 365;
    (*year)++;
  }

  // 월 계산
  *month = 1;
  while (total_days >= days_in_month[*month - 1] +
                           (*month == 2 && is_leap_year(*year) ? 1 : 0))
  {
    total_days -= days_in_month[*month - 1];
    if (*month == 2 && is_leap_year(*year))
    {
      total_days--;
    }
    (*month)++;
  }

  // 일 계산 (1일부터 시작)
  *day = total_days + 1;
}

bool is_valid_datetime(const DATE_TIME_BUF *nt)
{
  static const int8_t days_in_month[12] = {
      31, 28, 31, 30, 31, 30,
      31, 31, 30, 31, 30, 31};

  if (nt == NULL)
    return false;

  if (nt->Year < 1970 || nt->Year > 2099)
    return false;

  if (nt->Month < 1 || nt->Month > 12)
    return false;

  int8_t dim = days_in_month[nt->Month - 1];
  if (nt->Month == 2 && is_leap_year(nt->Year))
    dim = 29;

  if (nt->Day < 1 || nt->Day > dim)
    return false;

  if (nt->Hour < 0 || nt->Hour > 23)
    return false;

  if (nt->Min < 0 || nt->Min > 59)
    return false;

  if (nt->Sec < 0 || nt->Sec > 59)
    return false;

  if (nt->Week < 0 || nt->Week > 6)
    return false;

  if (nt->SubSec > 99)
    return false;

  return true;
}