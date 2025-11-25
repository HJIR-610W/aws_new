

#ifndef APP_LOGGING_H
#define APP_LOGGING_H

#include <stdint.h>

#include "logging_define.h"

#define LOG_COUNT_MAX 10000 



int32_t save_log(const char *log);
int32_t logging_read_log(uint32_t log_q_cnt,system_log_t *log_msg);
uint32_t logging_get_log_count(void);
void logging_init(void);
#endif
