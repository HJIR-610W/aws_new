
#ifndef TIME_DEFINE_H
#define TIME_DEFINE_H

#include <stdint.h>
typedef struct
{
	int16_t Year;
	int8_t Month;
	int8_t Day;
	int8_t Hour;
	int8_t Min;
	int8_t Sec;
	int8_t Week;		// 0~6, Sunday = 0
  uint16_t SubSec;//100th
} DATE_TIME_BUF;


#endif
