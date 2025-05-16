
#ifndef TASK_LOGGING_H
#define TASK_LOGGING_H

#include "utile_time.h"






#define LOGGING_LOG_ERR  0x01U
#define LOGGING_DATA_ERR 0x04U

typedef struct logging_task_info_s
{
  uint8_t status_group;
} logging_system_t;

typedef enum
{
  L_DEBUG = 0,//디버깅용 상세 정보 (출력 많음)
  L_INFO,//정상 흐름 정보 (상태, 동작)
  L_WARN,//비정상이지만 치명적이지 않음
  L_ERROR,//기능 실패 또는 복구 실패
  L_FATAL//시스템 치명적 오류 (재부팅 등 필요)
} log_level_t;


void loggingTask_init(void);
void log_printf(log_level_t level, const char *pFmt, ...);
void os_write_sensorData(DATE_TIME_BUF *pDate, void *pInData,uint32_t dataSize,
                         uint8_t Type,uint32_t periodMin);
logging_system_t *get_logging_system(void);


#endif