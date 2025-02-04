#define __STDC_WANT_LIB_EXT1__ 1

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"

#include "app_file.h"
#include "app_flash.h"
#include "app_logging.h"
#include "config.h"
#include "utile_time.h"




#define SYSTEM_NORM_MAX 1000
#define LOG_LEN 32 ////4(tick) + 1(code) + 27(msg null 포함)

static osSemaphoreId_t loggingSem;




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

 //050524224635,measure task       //

void logging_printf(const char * pFmt, ...)
{
    char buff[32];
    int len;
    uint16_t logCnt;
    uint32_t tick=0;;
    uint32_t totalBytes;
    va_list ap;  
    DATE_TIME_BUF ct;
    uint32_t cnt = 0;

    
    osSemaphoreAcquire(loggingSem, osWaitForever);

    logCnt = logging_get_logCnt();

    time_get(&ct);

    if(logCnt >= SYSTEM_NORM_MAX)
    {
        logCnt = 0;
    }

    memset_s(buff,sizeof(buff),0x00,sizeof(buff));
    //000102042914,measure_________
    len = snprintf_s(buff,sizeof(buff),"%02d%02d%02d%02d%02d%02d,",ct.Year%100,ct.Month,
    ct.Day,ct.Hour,ct.Min,ct.Sec);


    va_start(ap, pFmt);
    vsnprintf_s((char *)&buff[13], sizeof(buff)-13, (char *)pFmt, ap);

    va_end(ap);

    totalBytes = logCnt*LOG_LEN;// 저장된 로그 바이트 

#if 0
    flash_write(LOG_START_ADDRESS + totalBytes,(uint8_t *)buff,sizeof(buff));
#else
    write_file((char *)system_log_path,(uint8_t*)buff,sizeof(buff),totalBytes);
#endif
    logCnt++;
    logging_set_logCnt(logCnt);

     osSemaphoreRelease(loggingSem); 
}



void logging_read_log(int32_t offsetCnt,loggingMsg_t *loggingMsg)
{
  int zeroCnt=0;

    uint32_t totalBytes;


    osSemaphoreAcquire(loggingSem, osWaitForever);

    totalBytes = (offsetCnt-1)*LOG_LEN;

 

    read_file((char *)system_log_path,(uint8_t *)loggingMsg->msg,sizeof(loggingMsg->msg),totalBytes);

   
    for(int i = 0 ; i < sizeof(loggingMsg->msg);i++)
    {
      if(loggingMsg->msg[i] != ' ')
      {
      if(loggingMsg->msg[i] < 33 || loggingMsg->msg[i] > 126)
      {
          loggingMsg->msg[i] = 0;
      }
      }
      if(loggingMsg->msg[i]==0)
      {
        zeroCnt++;
        break;
      }
    }

  if(zeroCnt==0)
  {
    loggingMsg->msg[0]=0;
  }


    
     osSemaphoreRelease(loggingSem); 
}


void logging_init(void)
{
  loggingSem = osSemaphoreNew(1, 1, NULL);  
}