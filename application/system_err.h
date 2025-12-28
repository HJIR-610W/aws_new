
#ifndef SYSTEM_ERR_H
#define SYSTEM_ERR_H

#include <stdint.h>
#include <stdbool.h>
#include "util_time.h"
#include "debug_io.h"
#include "task_core_debug.h"

#define ERROR_PRINTF_USE // 시스템 에러 출력
#define DEBUG_PRINTF_USE// 디버깅 필요시
#define USE_DEBUG 0
#define IWDG_USE 1
#define PRINTF_BASE(fmt, ...)                                                               \
  log_printf("%04d-%02d-%02d %02d:%02d:%02d [%s] " fmt "\r\n",                      \
              Date_Time.Year, Date_Time.Month, Date_Time.Day,                               \
              Date_Time.Hour, Date_Time.Min, Date_Time.Sec,              \
              __FILE__, ##__VA_ARGS__)

#ifdef ERROR_PRINTF_USE
#define ERROR_PRINTF(fmt, ...) PRINTF_BASE(fmt, ##__VA_ARGS__)
#else
  #define ERROR_PRINTF(fmt, ...) ((void)0)
#endif

#define TASK_PRINTF(fmt, ...)                                               \
  task_printf("%04d-%02d-%02d %02d:%02d:%02d [%s] " fmt "\r\n",                      \
              Date_Time.Year, Date_Time.Month, Date_Time.Day,                               \
              Date_Time.Hour, Date_Time.Min, Date_Time.Sec,              \
              get_task_name(), ##__VA_ARGS__)

// 로그 레벨 정의
#define LOG_LEVEL_FATAL   1
#define LOG_LEVEL_ERROR   2
#define LOG_LEVEL_WARN    3
#define LOG_LEVEL_INFO    4
#define LOG_LEVEL_DEBUG   5
#define LOG_LEVEL_VERBOSE 6
#define LOG_LEVEL_NONE    99 

#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#define DEBUG_PRINTF_LEVEL(level, fmt, ...) \
    do {\
        if (level >= LOG_LEVEL_NONE) {}\
        else if (level <= CURRENT_LOG_LEVEL) { \
            PRINTF_BASE(fmt , ##__VA_ARGS__); \
        } \
    } while(0)



#ifdef DEBUG_PRINTF_USE
  #define DEBUG_PRINTF(fmt, ...)   log_printf(fmt "\r\n", ##__VA_ARGS__)
#else
  #define DEBUG_PRINTF(fmt, ...) ((void)0)
#endif


void Error_Handler(const char *file,int32_t line);
void reset_system(const char * pFmt, ...);
bool read_last_error(char *buffer, size_t len);
void reset_system_delay(uint32_t delay_seconds);
#endif
