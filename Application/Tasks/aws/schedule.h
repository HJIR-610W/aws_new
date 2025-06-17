
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "util_time.h"
#include "old_aws_define.h"



extern AWS_DATA_STRUCT mRealAws;   // 실시간 자료
extern AWS_DATA_STRUCT mMinAws;    // 1분 자료
extern AWS_DATA_STRUCT m10MinAws;  // 10분 자료
extern AWS_DATA_STRUCT mHourAws;   // 1 시간 자료

extern SYSTEM_INFO_AWS Sysinfo;


void AwsMinMaxInit(void);
void schedule_process(DATE_TIME_BUF *pDate, DATE_TIME_BUF *pOldDate);
uint8_t check_1min_data_updated(void);

SYSTEM_INFO_AWS *get_system_info_aws(void);

#endif