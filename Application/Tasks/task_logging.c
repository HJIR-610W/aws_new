

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#include "app_file.h"
#include "cmsis_os2.h"
#include "driver_rtc.h"
#include "dev_io.h"
#include "utile_time.h"
#include "app_logging.h"
#include "app_dataLogging.h"




const osThreadAttr_t loggingTask_attributes = {
  .name = "loggingTask",
  .stack_size = 2048,//2048바이트가 할당됨 하지만 4바이트 단위로 스택은 구성됨
  .priority = (osPriority_t) osPriorityNormal,
};


typedef enum logging_cmd_e
{
  eLOGGING_LOG,
  eLOGGING_DATA
}eLOGGING_CMD_t;


typedef struct logging_s
{
  eLOGGING_CMD_t cmd;
  DATE_TIME_BUF ct;
  uint16_t len;
  uint8_t data[200];
}logging_t;

osMessageQueueId_t loggingQueue;


void os_logging_printf(const char * pFmt, ...)
{
  logging_t logging;
  va_list ap;  


  va_start(ap, pFmt);
  vsnprintf((char *)&logging.data[0], sizeof(logging.data), (char *)pFmt, ap);
  va_end(ap);

    logging.cmd = eLOGGING_LOG;
  if(osMessageQueuePut(loggingQueue, &logging, 0, osWaitForever) != osOK)
  {

  }
}

void os_write_sensorData(DATE_TIME_BUF *pDate, void *pInData,uint32_t dataSize,
                         uint8_t Type,uint32_t periodMin)
{
  logging_t logging;

  logging.ct = *pDate;
  
  memcpy(&logging.data[0],&dataSize,4);
  memcpy(&logging.data[4],&Type,1);
  memcpy(&logging.data[5],&periodMin,4);

  memcpy(&logging.data[9],pInData,dataSize);

  logging.cmd = eLOGGING_DATA;
  if(osMessageQueuePut(loggingQueue, &logging, 0, osWaitForever) != osOK)
  {

  }
}


void loggingTask(void *arg)
{
  logging_t logging;
  DATE_TIME_BUF ct;
  uint32_t dataSize;
  uint8_t Type;
  uint32_t periodMin;

  while(1)
  {
    // 메시지 큐에서 데이터 수신
    if (osMessageQueueGet(loggingQueue, &logging, NULL, osWaitForever) == osOK)
    {
        switch(logging.cmd)
        {
          case eLOGGING_LOG:
          logging_printf((char *)"%s",&logging.data[0]);
          break;
          case eLOGGING_DATA:
            memcpy(&dataSize,&logging.data[0],sizeof(dataSize));
            memcpy(&Type,&logging.data[4],sizeof(Type));
            memcpy(&periodMin,&logging.data[5],sizeof(periodMin));
          write_sensorData(&logging.ct,&logging.data[9],dataSize,Type,periodMin);
          break;
        }
    }
  }
}





void loggingTask_init(void)
{
  loggingQueue = osMessageQueueNew(5, sizeof(logging_t), NULL);
   


   osMessageQueueId_t _atMailId;
osMessageQueueId_t _tcpAckMailId;
osMessageQueueId_t _asyncAckMailId;
osMessageQueueId_t _smsMailId;

osMessageQueueId_t _tcpDataMailId;
osMessageQueueId_t _callReqMailId;


  osThreadNew(loggingTask, NULL, &loggingTask_attributes);

}