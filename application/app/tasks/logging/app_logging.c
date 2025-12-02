
/*
app_logging.c

uint32_t log_q_cnt

로그는 최대 10000개 를 저장하고 순환방식으로 덮어쓰기한다.
단 log_q_cnt는 항상 1씩 증가 시킨다.

실제 로그 카운트는 log_q_cnt%10000개 

서버에서 현재 로그카운트가 변했는지 확인하기 위해 log_q_cnt를 읽고
이값이 이전과 다르면 새로운 로그가 생겼다고 판단한다.
초당 1개씩 로그가 쌓여도 수십년저장가능한 로그 수이다.

만약 서버 마지막 log_q_cnt 10001 
인데 다시 확인했을때 20001 이이면

장비입장에서는 한바뀌 덮어쓰기가 된 상태이다.

서버는 차이가 10000이상이면 한바퀴 돌았다고 생각하고 전체를 읽어야 한다.

*/

#include "app_logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "app_file.h"
#include "config_nvm.h"
#include "util_time.h"
#include "os_user_def.h"
#include "drv_flash.h"

#define LOG_SEM_ENABLE 0 

const char *kSystem_log_path = "0:System/log.txt";

#if (LOG_SEM_ENABLE==1)
static osSemaphoreId_t g_logging_sem;
#endif

uint32_t logging_get_log_count(void)
{
  return nvm_get_log_cnt();
}

void logging_set_log_count(uint32_t cnt)
{
  nvm_set_log_cnt(cnt);
}

//2015-05-24 22:46:35,measure task
/*
로그가 64바이트씩 저장되도록 한다.
*/
int32_t save_log(const char *log)
{
  char buff[LOG_LEN_MAX];
  int i=0;
  uint32_t index;
  uint32_t log_count;
  uint32_t total_bytes;

  int32_t err=0;

#if (LOG_SEM_ENABLE==1)
  osSemaphoreAcquire(g_logging_sem, osWaitForever);
#endif
  log_count = logging_get_log_count();

  index = log_count % LOG_COUNT_MAX;

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

    total_bytes = index * LOG_LEN_MAX;  // 저장된 로그 바이트

    err = write_file((char *)kSystem_log_path,(uint8_t*)buff,strlen(buff),total_bytes);
    if(err == 0)
    {
      log_count++;
      logging_set_log_count(log_count);
    }

#if (LOG_SEM_ENABLE==1)
    osSemaphoreRelease(g_logging_sem); 
#endif
    return err;
}

/**
 * @brief 저장된 로그 읽기
 * @param log_q_cnt 읽을 로그 번호
 * @param log_msg 로그 메시지
 * @note 2015-05-24 22:46:35,measure task
 */
int logging_read_log(uint32_t log_q_cnt, system_log_t *log_msg)
{
  int i;
  int fret;
  uint32_t offset;

  
#if (LOG_SEM_ENABLE==1)
  osSemaphoreAcquire(g_logging_sem, osWaitForever);
#endif
  memset(log_msg, 0, sizeof(system_log_t));

  offset = (log_q_cnt - 1) * LOG_LEN_MAX;

  fret = (int)read_file((char *)kSystem_log_path, (uint8_t *)log_msg->msg, sizeof(log_msg->msg),offset);

  if(fret !=0)
  {

#if (LOG_SEM_ENABLE==1)
    osSemaphoreRelease(g_logging_sem);
#endif
    return 1;
  }

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
#if (LOG_SEM_ENABLE==1)
  osSemaphoreRelease(g_logging_sem);
#endif

  return fret;
}


void logging_init(void)
{
#if (LOG_SEM_ENABLE==1)
  g_logging_sem = osSemaphoreNew(1, 1, NULL);  
#endif
}