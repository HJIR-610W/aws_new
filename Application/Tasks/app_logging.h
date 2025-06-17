

#ifndef APP_LOGGING_H
#define APP_LOGGING_H

#include <stdint.h>
#include "util_time.h"

#define LOG_LEN_MAX 45

#pragma pack(push, 1)
typedef struct 
{
  char msg[LOG_LEN_MAX];  // 문자열만 저장
}sysLog_t;
#pragma pack(pop)

int32_t save_log(const char *log);
int logging_read_log(uint32_t log_q_cnt,sysLog_t *loggingMsg);
uint16_t logging_get_logCnt(void);
void logging_init(void);
#endif
