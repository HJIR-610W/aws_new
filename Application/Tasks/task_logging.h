
#ifndef TASK_LOGGING_H
#define TASK_LOGGING_H

#include "utile_time.h"

#define IS_LOG_ERR()  ((g_loggingStatusGroup&LOGGING_LOG_ERR)>0)  
#define IS_DATA_ERR()  ((g_loggingStatusGroup&LOGGING_DATA_ERR)>0)  



#define LOGGING_LOG_ERR  0x01U
#define LOGGING_DATA_ERR 0x04U

void loggingTask_init(void);
void os_logging_printf(const char * pFmt, ...);
void os_write_sensorData(DATE_TIME_BUF *pDate, void *pInData,uint32_t dataSize,
                         uint8_t Type,uint32_t periodMin);

extern uint8_t g_loggingStatusGroup;


#endif