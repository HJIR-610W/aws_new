

#include "cmsis_os.h"
#include "driver_rtc.h"
#include "io.h"
#include "lwip.h"
#include "utile_time.h"


const osThreadAttr_t tcpServerTask_attributes = {
  .name = "tcpServerTask",
  .stack_size = 2048,//2048바이트가 할당됨 하지만 4바이트 단위로 스택은 구성됨
  .priority = (osPriority_t) osPriorityNormal,
};





void tcpServerTask(void *arg)
{

  while(1)
  {

    osDelay(1000);

  }
}



void tcpServerTask_init(void)
{
  osThreadNew(tcpServerTask, NULL, &tcpServerTask_attributes);
}
