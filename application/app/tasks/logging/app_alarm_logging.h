

#ifndef APP_ALARM_LOGGING_H
#define APP_ALARM_LOGGING_H

#include <stdint.h>
#include "logging_define.h"


#define ALARM_LOG_COUNT_MAX 10000 



int32_t alarm_save_log(const char *log);
int32_t alarm_read_log(uint32_t log_q_cnt,system_log_t *log_msg);
uint32_t alarm_get_log_count(void);
void alarm_logging_init(void);
#endif
