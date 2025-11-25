
#include "app_alarm_logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "app_file.h"
#include "config_nvm.h"
#include "util_time.h"
#include "os_user_def.h"
#include "drv_fram.h"
#include "drv_flash.h"

#define ALARM_LOG_SEM_ENABLE 0 //싱글 task에서만 사용되기때문에 불필요



#if (ALARM_LOG_SEM_ENABLE==1)
static osSemaphoreId_t g_alarm_sem;
#endif

uint32_t alarm_get_log_count(void)
{
  return nvm_get_alarm_count();
}

void alarm_set_log_count(uint32_t cnt)
{
  nvm_set_alarm_count(cnt);
}

//2015-05-24 22:46:35,measure task
/*
로그가 64바이트씩 저장되도록 한다.
*/
int32_t alarm_save_log(const char *log)
{
  char buff[LOG_LEN_MAX];
  int i=0;
  uint32_t index;
  uint32_t log_count;
  uint32_t offset;

  int32_t err=0;

#if (ALARM_LOG_SEM_ENABLE==1)
  osSemaphoreAcquire(g_alarm_sem, osWaitForever);
#endif
  log_count = alarm_get_log_count();

  index = log_count % ALARM_LOG_COUNT_MAX;

  memset(buff, 0x00, sizeof(buff));

  for (i = 0; i < sizeof(buff) - 1; i++)
  {
    if (*log)
    {
      buff[i] = *log++;
    }
    else
    {
      buff[i] = ' ';
    }
  }

    buff[sizeof(buff)-1]=0;//마지막 NULL 처리리

    offset = index * LOG_LEN_MAX;  // 저장된 로그 바이트

    drv_flash_write(offset,(uint8_t *)buff,sizeof(buff));

    log_count++;
    alarm_set_log_count(log_count);


#if (ALARM_LOG_SEM_ENABLE==1)
    osSemaphoreRelease(g_alarm_sem); 
#endif
    return err;
}

/**
 * @brief 저장된 로그 읽기
 * @param log_q_cnt 읽을 로그 번호
 * @param log_msg 로그 메시지
 * @note 2015-05-24 22:46:35,measure task
 */
int alarm_read_log(uint32_t log_q_cnt, system_log_t *log_msg)
{
  int i;
  int fret = 0;
  uint32_t offset;

  
#if (ALARM_LOG_SEM_ENABLE==1)
  osSemaphoreAcquire(g_alarm_sem, osWaitForever);
#endif
  memset(log_msg, 0, sizeof(system_log_t));

  offset = (log_q_cnt - 1) * LOG_LEN_MAX;

  drv_flash_read(offset,(uint8_t*)log_msg->msg,sizeof(log_msg->msg));


#if (ALARM_LOG_SEM_ENABLE==1)
    osSemaphoreRelease(g_alarm_sem);
#endif

  // readable하지 않은 문자를 공백으로 처리
  for(i = 0 ; i < sizeof(log_msg->msg); i++)
  {
    // printable ASCII 범위 체크 (0x20~0x7E)
    if(log_msg->msg[i] < 0x20 || log_msg->msg[i] > 126)
    {
      log_msg->msg[i] = ' ';
    }
  }

  log_msg->msg[sizeof(log_msg->msg) - 1] = 0;
#if (ALARM_LOG_SEM_ENABLE==1)
  osSemaphoreRelease(g_alarm_sem);
#endif

  return fret;
}


void alarm_logging_init(void)
{
#if (ALARM_LOG_SEM_ENABLE==1)
  g_alarm_sem = osSemaphoreNew(1, 1, NULL);  
#endif
}