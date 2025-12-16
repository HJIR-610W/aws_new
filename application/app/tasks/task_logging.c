

#include "task_logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "logging\save_csv.h"
#include "logging\app_dataLogging.h"
#include "logging\app_logging.h"
#include "logging\app_alarm_logging.h"
#include "logging\save_csv.h"

#include "app_file.h"
#include "debug_io.h"
#include "fatfs.h"
#include "old_aws_define.h"
#include "os_user_def.h"
#include "system_err.h"
#include "util_time.h"
#include "task_menu.h"
#include "task_wdt.h"
#include "config_app.h"



typedef enum logging_cmd_e
{
  eLOGGING_LOG,      // 로깅 task로 로그를 전송 할 때 사용
  eLOGGING_DATA,      // 로깅 task로 데이터를 전송 할 때 사용
  eLOGGING_RAIN 
} eLOGGING_CMD_t;

typedef struct logging_s
{
  char data[300];
  uint16_t len;
  eLOGGING_CMD_t cmd;
  DATE_TIME_BUF ct;
}logging_t;

const osThreadAttr_t kLoggingTask_attributes = {
  .name = "logging",
  .stack_size = TASK_STACK(TASK_LOGGING_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_LOGGING_DEF),
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
    debug_printf("log_printf_level timeout.\r\n");
  }
}


void log_out_uart(logging_t *logging)
{
  debug_printf("%s",logging->data);
}

void log_printf(log_level_t level, const char *pFmt, ...)
{
  logging_t logging;
  va_list ap;
  int32_t len = 0;
  DATE_TIME_BUF ct;

  if (level < g_log_level || g_log_output == NULL)
  return;

  ct = Date_Time;


  len = snprintf(logging.data, sizeof(logging.data),
                 "%04d-%02d-%02d %02d:%02d:%02d,", 
                 ct.Year, ct.Month, ct.Day,
                 ct.Hour, ct.Min, ct.Sec);

  if (len < 0 || len >= LOG_LEN_MAX) return;


  const char *level_str = log_level_str(level);
  len += snprintf(&logging.data[len], sizeof(logging.data) - len,
                  "%s,", level_str);


  va_start(ap, pFmt);
  int msg_len = vsnprintf(&logging.data[len],
                          sizeof(logging.data) - len, pFmt, ap);
  va_end(ap);



  len +=msg_len;
  
  for (int i = len; i < LOG_LEN_MAX; i++) {
    logging.data[i] = ' ';
  }


  logging.data[LOG_LEN_MAX-2] = '\r';
  logging.data[LOG_LEN_MAX-1] = '\n';

  logging.cmd = eLOGGING_LOG;

 g_log_output(&logging); 
}





void os_save_aws_data(DATE_TIME_BUF *pDate, void *pInData,uint32_t data_size, uint8_t type,uint32_t period_min)
{
  logging_t logging;
  data_logging_cmd_t *p_frame;

  logging.ct = *pDate;
  logging.cmd = eLOGGING_DATA;

  p_frame = (data_logging_cmd_t *)logging.data;
  p_frame->data_len = data_size;
  p_frame->type = type;
  p_frame->period_min = period_min;
  memcpy(&p_frame->data[0],pInData,data_size);

  if(osMessageQueuePut(g_loggingQueue, &logging, 0, kLoggingTimeOutMs) != osOK)
  {
    debug_printf("os_save_aws_data timeout.\r\n");
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
  int32_t err=0;
  logging_t logging;
  uint16_t rain;
  data_logging_cmd_t *p_frame;
  uint32_t offset;
  uint16_t sunshine;
  int32_t wdt_number;

  DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"logging task start\r\n");

  //이 task는 최소 1분에 한번씩 호출되어야한다.
  wdt_number = wdt_task_register(kLoggingTask_attributes.name, 90000);

  while (1)
  {
    // 메시지 큐에서 데이터 수신
    if (osMessageQueueGet(g_loggingQueue, &logging, NULL, osWaitForever) == osOK)
    {
         switch(logging.cmd)
        {
          case eLOGGING_LOG:
            if(strncmp(&logging.data[20],"FATL",4)==0||strncmp(&logging.data[20],"ERRO",4)==0)
            {
              err = alarm_save_log((char *)logging.data);
            }
            else
            {
              err = save_log((char *)logging.data);
            }
            if(err)
            {
              debug_printf("log err:%d\r\n",err);
            }
            break;
          case eLOGGING_DATA:
            p_frame = (data_logging_cmd_t *)logging.data;
            err = write_data_month(&logging.ct, &p_frame->data[0], p_frame->data_len, p_frame->type, p_frame->period_min);
            update_loggingErr(&g_logging_system.status_group, err, LOGGING_DATA_ERR);
            if (p_frame->type == LOGGING_AWS)
            {
              //1분 우량만 별도의 파일에 저장
              offset = OFFSET_OF_RAIN();
              memcpy(&rain, &p_frame->data[offset], sizeof(uint16_t));
              err = write_rain_1min(&logging.ct, rain);
              update_loggingErr(&g_logging_system.status_group, err, LOGGING_RAIN_ERR);
              //1분 일조만 별도의 파일 저장 
              offset = OFFSET_OF_SUN();
              memcpy(&sunshine, &p_frame->data[offset], sizeof(uint16_t));
              err = write_sunshine_1min(&logging.ct, sunshine);
              update_loggingErr(&g_logging_system.status_group, err, LOGGING_SUN_ERR);
            }
            if(config.aws_csv_save_active)
            {
              save_aws_csv(p_frame->data,p_frame->data_len);
            }
            
            break;
        }

    }

    wdt_task_feed(wdt_number);
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