

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"


#include "pcb_define.h"
#include "config.h"
#include "driver_rtc.h"


#include "app_file.h"
#include "app_logging.h"
#include "app_dataLogging.h"
#include "dev_io.h"
#include "utile_time.h"
#include "task_logging.h"


typedef enum logging_cmd_e
{
  eLOGGING_LOG, //로깅 task로 로그를 전송 할 때 사용
  eLOGGING_DATA //로깅 task로 데이터를 전송 할 때 사용
}eLOGGING_CMD_t;



typedef struct logging_s
{
  eLOGGING_CMD_t cmd;
  DATE_TIME_BUF ct;
  uint16_t len;
  uint8_t data[300];
}logging_t;

const osThreadAttr_t kLoggingTask_attributes = {
  .name = "loggingTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

const uint32_t kLoggingTimeOutMs = 50;

osMessageQueueId_t g_loggingQueue;
uint8_t g_loggingStatusGroup;

/**
 * @brief 시스템 로깅
 */
void os_logging_printf(const char * pFmt, ...)
{
  logging_t logging;
  va_list ap;  
  int32_t len;
  int32_t i;

  DATE_TIME_BUF ct;
  
  time_get(&ct);
  //2000-01-01 00:00:00,{문자열}     \r\n\0

  len = snprintf(logging.data,sizeof(logging.data),"%04d-%02d-%02d %02d:%02d:%02d,",ct.Year,ct.Month,ct.Day,
  ct.Hour,ct.Min,ct.Sec);

  va_start(ap, pFmt);
  len += vsnprintf((char *)&logging.data[len], sizeof(logging.data)-len, (char *)pFmt, ap);
  va_end(ap);

  //
  for(int i = len; i <= 60;i++)
  {
    logging.data[i]= ' ';//
  }

  logging.data[61] = '\r';
  logging.data[62] = '\n';
  logging.data[63] = '\0';


  logging.cmd = eLOGGING_LOG;//로그는 문자열만 전송송

  if(osMessageQueuePut(g_loggingQueue, &logging, 0, kLoggingTimeOutMs) != osOK)
  {
    debug_printf("os_logging_printf timeout.\r\n");
  }
}

void os_write_sensorData(DATE_TIME_BUF *pDate, void *pInData,uint32_t data_size,
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
    debug_printf("os_write_sensorData timeout.\r\n");
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

  while(1)
  {
    // 메시지 큐에서 데이터 수신
    if (osMessageQueueGet(g_loggingQueue, &logging, NULL, osWaitForever) == osOK)
    {
        switch(logging.cmd)
        {
          case eLOGGING_LOG:
            err = logging_printf((char *)logging.data);
            update_loggingErr(&g_loggingStatusGroup,err,LOGGING_LOG_ERR);
          break;
          case eLOGGING_DATA:
            memcpy(&data_size,&logging.data[0],sizeof(data_size));
            memcpy(&data_type,&logging.data[4],sizeof(data_type));
            memcpy(&period_min,&logging.data[5],sizeof(period_min));
            err =write_data(&logging.ct,&logging.data[9],data_size,data_type,period_min);
            update_loggingErr(&g_loggingStatusGroup,err,LOGGING_DATA_ERR);
          break;
        }

    }
  }
}




void loggingTask_init(void)
{
  /*
   로그가 동시에 전송될것을 고려하여 적당한 갯수 필요
   큐가 부족하면 로그 저장이 안될 수 있음
  */
  g_loggingQueue = osMessageQueueNew(5, sizeof(logging_t), NULL);

  assert_param(g_loggingQueue);

  osThreadNew(loggingTask, NULL, &kLoggingTask_attributes);

}