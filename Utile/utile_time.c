#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>
#include <stdio.h>

#include "cmsis_os.h"
#include "utile_time.h"

DATE_TIME_BUF Date_Time;


void time_get(DATE_TIME_BUF *ct)
{
  portDISABLE_INTERRUPTS();
  *ct = Date_Time;
  portENABLE_INTERRUPTS();
}

void time_set(DATE_TIME_BUF *nt)
{
  portDISABLE_INTERRUPTS();
  Date_Time = *nt; //
  portENABLE_INTERRUPTS();
}

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

    int32_t make_timeToStr(DATE_TIME_BUF *ct, char *out, uint16_t outSize)
{
  return snprintf_s(out,outSize,"%04d-%02d-%02d %02d:%02d:%02d",Date_Time.Year,
  Date_Time.Month,Date_Time.Day,Date_Time.Hour,Date_Time.Min,Date_Time.Sec);
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

bool isLeapYear(int year) { return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0); }
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

  const uint16_t *table = isLeapYear(year) ? days_until_month_leap : days_until_month;
  return table[month - 1] + day;
}

void subtract_seconds(DATE_TIME_BUF *dt, uint32_t seconds)
{
  uint32_t tick = time_cvt_timestamp(dt);

  tick = tick -  seconds;

  time_cvt_secTotime(tick,dt);

}