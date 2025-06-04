
#include "app_logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"

#include "app_file.h"
#include "config_nvm.h"
#include "util_time.h"



const char *kSystem_log_path = "0:System/log.txt";
const uint16_t kSystemNormMax = 10000;

static osSemaphoreId_t g_loggingSem;

uint16_t logging_get_logCnt(void)
{
  return nvm_get_log_cnt();
}

void logging_set_logCnt(uint16_t cnt)
{
  nvm_set_log_cnt(cnt);
}

//2015-05-24 22:46:35,measure task
/*
로그가 64바이트씩 저장되도록 한다.
*/
int32_t logging_printf(const char *log)
{
  char buff[LOG_LEN_MAX];
  int len;
  int i=0;
  uint16_t logCnt;
  uint32_t totalBytes;
  uint32_t cnt = 0;
  int32_t err=0;

  osSemaphoreAcquire(g_loggingSem, osWaitForever);

  logCnt = logging_get_logCnt();

  if(logCnt >= kSystemNormMax)
  {
    logCnt = 0;
  }

    memset(buff,0x00,sizeof(buff));

    for( i = 0 ; i < sizeof(buff)-1;i++)
    {
      if(*log)
      {
        buff[i] = *log++;
      }
      else
      {

        buff[i]=' ';
      }
    }

    buff[sizeof(buff)-1]=0;//마지막 NULL 처리리
    
    totalBytes = logCnt*LOG_LEN_MAX;// 저장된 로그 바이트 

    err = write_file((char *)kSystem_log_path,(uint8_t*)buff,sizeof(buff),totalBytes);

    logCnt++;
    logging_set_logCnt(logCnt);

    osSemaphoreRelease(g_loggingSem); 

    return err;
}

void logging_read_log(uint32_t log_q_cnt, loggingMsg_t *loggingMsg)
{
  int zeroCnt=0;

  uint32_t totalBytes;

  osSemaphoreAcquire(g_loggingSem, osWaitForever);

  totalBytes = (log_q_cnt - 1) * LOG_LEN_MAX;

  read_file((char *)kSystem_log_path,(uint8_t *)loggingMsg->msg,sizeof(loggingMsg->msg),totalBytes);

  for(int i = 0 ; i < sizeof(loggingMsg->msg);i++)
  {
    if(loggingMsg->msg[i] != ' ')
    {
       //스페이스미만
      if(loggingMsg->msg[i] <0x20 || loggingMsg->msg[i] > 126)
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
  
  osSemaphoreRelease(g_loggingSem); 
}


void logging_init(void)
{
  g_loggingSem = osSemaphoreNew(1, 1, NULL);  
}