

#include "cmsis_os.h"
#include "driver_rtc.h"
#include "dev_io.h"
#include "utile_time.h"


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





void loggingTask(void *arg)
{

  while(1)
  {
  osDelay(1000);

  }
}



void loggingTask_init(void)
{
   
  osThreadNew(loggingTask, NULL, &loggingTask_attributes);

}