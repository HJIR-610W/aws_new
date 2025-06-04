

#include "task_logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "app_dataLogging.h"
#include "app_file.h"
#include "app_logging.h"

#include "old_aws_define.h"
#include "util_time.h"
#include "dev_io.h"
typedef enum logging_cmd_e
{
  eLOGGING_LOG,      // 로깅 task로 로그를 전송 할 때 사용
  eLOGGING_DATA,      // 로깅 task로 데이터를 전송 할 때 사용
  eLOGGING_RAIN 
} eLOGGING_CMD_t;

typedef struct logging_s
{
  eLOGGING_CMD_t cmd;
  DATE_TIME_BUF ct;
  uint16_t len;
  char data[300];
}logging_t;

const osThreadAttr_t kLoggingTask_attributes = {
  .name = "loggingTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

const uint32_t kLoggingTimeOutMs = 50;

osMessageQueueId_t g_loggingQueue;
logging_system_t g_logging_system;
logging_system_t *get_logging_system(void)
{
  return &g_logging_system;
}

static const char *log_level_str(log_level_t level)
{
  switch (level)
  {
    case L_DEBUG: return "DEBU";
    case L_INFO:  return "INFO";
    case L_WARN:  return "WARN";
    case L_ERROR: return "ERRO";
    case L_FATAL: return "FATL";
    default:      return "UNKN";
  }
}
static log_level_t g_log_level = L_DEBUG;
static void (*g_log_output)(logging_t *logging) = NULL;

void log_set_level(log_level_t level) {
  g_log_level = level;
}

void log_set_output(void (*func)(logging_t *logging)) {
  g_log_output = func;
}

void log_out_queue(logging_t *logging)
{
  if(osMessageQueuePut(g_loggingQueue, logging, 0, kLoggingTimeOutMs) != osOK)
  {
    io_printf("log_printf_level timeout.\r\n");
  }
}


void log_out_uart(logging_t *logging)
{
  io_printf("%s",logging->data);
}

void log_printf(log_level_t level, const char *pFmt, ...)
{
  logging_t logging;
  va_list ap;
  int32_t len = 0;
  DATE_TIME_BUF ct;

  if (level < g_log_level || g_log_output == NULL)
  return;

  time_get(&ct);

  // 1. 날짜/시간
  len = snprintf(logging.data, sizeof(logging.data),
                 "%04d-%02d-%02d %02d:%02d:%02d,", 
                 ct.Year, ct.Month, ct.Day,
                 ct.Hour, ct.Min, ct.Sec);

  if (len < 0 || len >= 64) return;

  // 2. 로그 레벨 추가 
  const char *level_str = log_level_str(level);
  len += snprintf(&logging.data[len], sizeof(logging.data) - len,
                  "%s,", level_str);

  // 3. 메시지
  va_start(ap, pFmt);
  int msg_len = vsnprintf(&logging.data[len],
                          sizeof(logging.data) - len, pFmt, ap);
  va_end(ap);

  len += (msg_len > 0) ? msg_len : 0;
  if (len > 60) len = 60;

  // 4. 공백 패딩
  for (int i = len; i < 61; i++) {
    logging.data[i] = ' ';
  }

  // 5. 종료 처리
  logging.data[61] = '\r';
  logging.data[62] = '\n';
  logging.data[63] = '\0';

  logging.cmd = eLOGGING_LOG;

 g_log_output(&logging); 
}


void os_write_data_year(DATE_TIME_BUF *pDate, void *pInData,uint32_t data_size,
                         uint8_t Type,uint32_t period_min)
{
  logging_t logging;

  logging.ct = *pDate;
  
  memcpy(&logging.data[0],&data_size,4);
  memcpy(&logging.data[4],&Type,1);
  memcpy(&logging.data[5],&period_min,4);

  memcpy(&logging.data[9],pInData,data_size);

  logging.cmd = eLOGGING_DATA;
  if(osMessageQueuePut(g_loggingQueue, &logging, 0, kLoggingTimeOutMs) != osOK)
  {
    io_printf("os_write_data_year timeout.\r\n");
  }
}



void update_loggingErr(uint8_t *status,int8_t err,uint8_t flag)
{
  if(err)
  {
    *status |= flag;
  }
  else
  {
    *status &= ~flag;
  }
}


int32_t write_rain_1min(DATE_TIME_BUF *nt,uint16_t rain_1min)
{
  int32_t err;
  err = write_data_year(nt, &rain_1min, 2, LOGGING_RAIN_1MIN, 1);

  return err;
}

int32_t write_sunshine_1min(DATE_TIME_BUF *nt, uint16_t sunshine_1min)
{
  int32_t err;
  err = write_data_year(nt, &sunshine_1min, sizeof(uint16_t), LOGGING_SUNSHINE_1MIN, 1);

  return err;
}

#define OFFSET_OF_SUN() (uint32_t)(&(((AWS_DATA_STRUCT *)0)->mSunshine.sReal))
#define OFFSET_OF_RAIN() (uint32_t)(&(((AWS_DATA_STRUCT *)0)->rain_1min))

/**
 * @brief SD쓰기 처리리
 */
void loggingTask(void *arg)
{
  uint8_t data_type;
  int32_t err=0;
  uint32_t data_size;
  uint32_t period_min;
  logging_t logging;
  uint16_t rain;
  AWS_DATA_STRUCT *p_aws;

  uint32_t offset;
  uint16_t sunshine;
  while (1)
  {
    // 메시지 큐에서 데이터 수신
    if (osMessageQueueGet(g_loggingQueue, &logging, NULL, osWaitForever) == osOK)
    {
        switch(logging.cmd)
        {
          case eLOGGING_LOG:
            err = save_log((char *)logging.data);
            if(err)
            {
              io_printf("log err:%d\r\n",err);
            }
            break;
          case eLOGGING_DATA:
            memcpy(&data_size,&logging.data[0],sizeof(data_size));
            memcpy(&data_type,&logging.data[4],sizeof(data_type));
            memcpy(&period_min,&logging.data[5],sizeof(period_min));

            err = write_data_month(&logging.ct,&logging.data[9],data_size,data_type,period_min);
            update_loggingErr(&g_logging_system.status_group, err, LOGGING_DATA_ERR);
            offset = OFFSET_OF_RAIN();
            memcpy(&rain, &logging.data[9 + offset], sizeof(uint16_t));  
            err = write_rain_1min(&logging.ct, rain);
            update_loggingErr(&g_logging_system.status_group, err, LOGGING_RAIN_ERR);
            offset = OFFSET_OF_SUN();
            memcpy(&sunshine, &logging.data[9 + offset], sizeof(uint16_t));
            err = write_sunshine_1min(&logging.ct, sunshine);
            update_loggingErr(&g_logging_system.status_group, err, LOGGING_RAIN_ERR);
            break;
        }

    }
  }
}


void loggingTask_init(void)
{
  log_set_output(log_out_queue);
  /*
   로그가 동시에 전송될것을 고려하여 적당한 갯수 필요
   큐가 부족하면 로그 저장이 안될 수 있음
  */
  g_loggingQueue = osMessageQueueNew(5, sizeof(logging_t), NULL);

  assert_param(g_loggingQueue);

  osThreadNew(loggingTask, NULL, &kLoggingTask_attributes);

}