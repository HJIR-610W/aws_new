

#ifndef APP_LOGGING_H
#define APP_LOGGING_H

#include <stdint.h>


#define LOG_COUNT_MAX 10000 

#define LOG_LEN_MAX 64

#pragma pack(push, 1)
typedef struct 
{
  char msg[LOG_LEN_MAX];  // 문자열만 저장
}system_log_t; 
#pragma pack(pop)

int32_t save_log(const char *log);
int32_t logging_read_log(uint32_t log_q_cnt,system_log_t *log_msg);
uint32_t logging_get_log_count(void);
void logging_init(void);
#endif
