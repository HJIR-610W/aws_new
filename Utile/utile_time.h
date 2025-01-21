
#ifndef UTILE_TIME_H

#define UTILE_TILE_H


#include "time_define.h"

#include <time.h>
void time_cvt_secTotime(time_t sec,DATE_TIME_BUF *timeNow);
void time_get(DATE_TIME_BUF *ct);
void time_set(DATE_TIME_BUF *nt);
int32_t make_timeToStr(DATE_TIME_BUF *ct,char *out,uint16_t outSize);

extern DATE_TIME_BUF Date_Time;;
#endif
