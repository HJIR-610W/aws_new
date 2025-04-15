
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "utile_time.h"
#include "old_aws_define.h"

void ScheduleTask_init(void);

extern AWS_DATA_STRUCT mRealAws;   // 실시간 자료
extern AWS_DATA_STRUCT mMinAws;    // 1분 자료
extern AWS_DATA_STRUCT m10MinAws;  // 10분 자료
extern AWS_DATA_STRUCT mHourAws;   // 1 시간 자료

extern SYSTEM_INFO_AWS Sysinfo;
extern SYSTEM_CONFIG_AWS Config;

 void AwsMinMaxInit(void);

 void schedule_process(DATE_TIME_BUF *pDate);

#endif