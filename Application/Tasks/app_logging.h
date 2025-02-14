

#ifndef APP_LOGGING_H
#define APP_LOGGING_H

#include <stdint.h>
#include "utile_time.h"
typedef struct 
{
    char msg[32];
}loggingMsg_t;

int32_t logging_printf(const char * pFmt, ...);
void logging_read_log(int32_t offsetCnt,loggingMsg_t *loggingMsg);
void logging_init(void);
uint16_t logging_get_logCnt(void);
#endif
