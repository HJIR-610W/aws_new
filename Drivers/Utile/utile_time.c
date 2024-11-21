#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>

#include "utile_time.h"



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