

#ifndef LOGGING_DEFINE_H
#define LOGGING_DEFINE_H

#include <stdint.h>


#define LOG_LEN_MAX 64

#pragma pack(push, 1)
typedef struct 
{
  char msg[LOG_LEN_MAX];  // 문자열만 저장
}system_log_t; 
#pragma pack(pop)

#endif
