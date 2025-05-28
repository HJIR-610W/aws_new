
#ifndef UTILE_TIME_H
#define UTILE_TILE_H


#include <time.h>


#include "time_define.h"
#include <stdbool.h>


void time_cvt_secTotime(time_t sec,DATE_TIME_BUF *timeNow);
time_t time_cvt_timestamp(DATE_TIME_BUF *tN);
void time_get(DATE_TIME_BUF *ct);
void time_set(DATE_TIME_BUF *nt);
int32_t make_timeToStr(DATE_TIME_BUF *ct,char *out,uint16_t outSize);
time_t SetTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec);
int GetYear(time_t tmIn);
int GetDay(time_t tmIn);
long GetTotalSeconds(time_t ts);
int GetMonth(time_t tmIn);
void subtract_seconds(DATE_TIME_BUF *dt, uint32_t seconds);

    time_t time_timestamp(void);

bool isLeapYear(int year);
int dayOfYear(int year, int month, int day);

extern DATE_TIME_BUF Date_Time;;

#endif
