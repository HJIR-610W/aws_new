





#include "cmsis_os.h"




typedef struct cmd_s
{
  uint8_t cmd;
  uint8_t data[512];
}cmd_t;



osThreadId_t g_cmdTaskHandle;
osMessageQueueId_t g_cmdMessageQueue;


const osThreadAttr_t cmdTask_attributes = {
  .name = "cmdTask",
  .stack_size = 128 * 8,
  .priority = (osPriority_t) osPriorityNormal,
};


void put_cmd(uint8_t cmd,uint8_t data)
{
  if (osMessageQueuePut(messageQueue, &count, 0, osWaitForever) == osOK)
  {
  //    printf("Producer: Sent %lu to queue\n", count);
      count++;  // 다음 값 준비
  } else {
      printf("Producer: Failed to send message\n");
  }
}


void cmdTask(void *argument)
{
  
  while(1)
  {
      
  }
}





void cmdTask_init(void)
{
  
  g_cmdMessageQueue = osMessageQueueNew(1, sizeof(cmd_t), NULL);
  
  if(g_cmdMessageQueue == NULL)
  {
    
  }
  
  g_cmdTaskHandle = osThreadNew(cmdTask, NULL, &cmdTask_attributes);
}