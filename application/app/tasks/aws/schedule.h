
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "util_time.h"
#include "old_aws_define.h"

#define MIN1_PROC 0
#define MIN10_PROC 1
#define HOUR_PROC 2

extern AWS_DATA_STRUCT mRealAws;   // 실시간 자료
extern AWS_DATA_STRUCT mMinAws;    // 1분 자료
extern AWS_DATA_STRUCT m10MinAws;  // 10분 자료
extern AWS_DATA_STRUCT mHourAws;   // 1 시간 자료
extern AWS_DATA_STRUCT mDayAws; // 일간 자료 


void schedule_process(DATE_TIME_BUF *pDate, DATE_TIME_BUF *pOldDate);




#endif