
#ifndef SYSTEM_ERR_H
#define SYSTEM_ERR_H

#include <stdint.h>
#include <stdbool.h>
#include "util_time.h"
#define ERROR_PRINTF_USE 1 //HAL 에러 출력

#if ERROR_PRINTF_USE
#define ERROR_PRINTF(fmt, ...)                                                              \
  error_print("%04d-%02d-%02d %02d:%02d:%02d.%02d [%s:%d] " fmt "\r\n", Date_Time.Year,     \
              Date_Time.Month, Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec, \
              Date_Time.SubSec, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define ERROR_PRINTF(fmt, ...) ((void)0)
#endif

void Error_Handler(const char *file,int32_t line);
void reset_system(const char * pFmt, ...);
bool restore_error(char *p_out, int32_t out_size);
void error_print(const char *pFmt, ...);

#endif
