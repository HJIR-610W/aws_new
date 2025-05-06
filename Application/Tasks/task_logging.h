
#ifndef TASK_LOGGING_H
#define TASK_LOGGING_H

#include "utile_time.h"





#define LOGGING_LOG_ERR  0x01U
#define LOGGING_DATA_ERR 0x04U

typedef struct logging_task_info_s
{
  uint8_t status_group;
} logging_system_t;



void loggingTask_init(void);
void os_logging_printf(const char * pFmt, ...);
void os_write_sensorData(DATE_TIME_BUF *pDate, void *pInData,uint32_t dataSize,
                         uint8_t Type,uint32_t periodMin);
logging_system_t *get_logging_system(void);


#endif