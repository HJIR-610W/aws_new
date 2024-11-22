
#ifndef UTILE_TIME_H

#define UTILE_TILE_H


#include "time_define.h"

#include <time.h>
void time_cvt_secTotime(time_t sec,DATE_TIME_BUF *timeNow);
void time_get(DATE_TIME_BUF *ct);
void time_set(DATE_TIME_BUF *nt);
#endif
