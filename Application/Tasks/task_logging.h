
#ifndef TASK_LOGGING_H
#define TASK_LOGGING_H

#include "utile_time.h"

void loggingTask_init(void);
void os_logging_printf(const char * pFmt, ...);
void os_write_sensorData(DATE_TIME_BUF *pDate, void *pInData,uint32_t dataSize,
                         uint8_t Type,uint32_t periodMin);
#endif