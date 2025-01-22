#define __STDC_WANT_LIB_EXT1__ 1

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "app_flash.h"
#include "config.h"
#include "utile_time.h"



#define SYSTEM_NORM_MAX 1000
#define LOG_LEN 32 ////4(tick) + 1(code) + 27(msg null 포함)
uint16_t logging_get_logCnt(void)
{
    uint16_t cnt;
    cnt = config.logCnt;
    return cnt;
}

void logging_set_logCnt(uint16_t cnt)
{
  
  config.logCnt = cnt;
  WRITE_CFG(logCnt);
}


void logging_printf(uint8_t type,uint8_t code,const char * pFmt, ...)
{
    char buff[32];
    uint16_t logCnt;
    uint32_t tick=0;;
    uint32_t totalBytes;
    va_list ap;  
    DATE_TIME_BUF ct;
    uint32_t cnt = 0;

    

    logCnt = logging_get_logCnt();

    time_get(&ct);

    if(logCnt >= SYSTEM_NORM_MAX)
    {
        logCnt = 0;
    }

    memset_s(buff,sizeof(buff),0x00,sizeof(buff));
    
    tick = (uint32_t)time_cvt_timestamp(&ct);

    memcpy_s(&buff[0],sizeof(buff),&tick,sizeof(tick));
    cnt += sizeof(tick);
    buff[cnt] = (char)code;

    cnt += sizeof(code);

    va_start(ap, pFmt);
    vsnprintf_s((char *)&buff[cnt], sizeof(buff)-cnt, (char *)pFmt, ap);

    va_end(ap);

    totalBytes = logCnt*LOG_LEN;// 저장된 로그 바이트 


    flash_write(LOG_START_ADDRESS + totalBytes,(uint8_t *)buff,sizeof(buff));

    logCnt++;
    logging_set_logCnt(logCnt);
}