


#include <stdint.h>

#include "cmsis_os.h"
#include "task_cmd.h"
#define HOST_CMD_READ_SYSTEM 0x01
#define HOST_CMD_READ_CONFIG 0x02
#define HOST_CMD_WRITE_CONFIG 0x03
#define HOST_CMD_DOWNLOAD_FW 0x04
#define HOST_CMD_UPDATE_FW   0x05



osThreadId_t g_cmdTaskHandle;
osMessageQueueId_t g_cmdMessageQueue;


const osThreadAttr_t cmdTask_attributes = {
  .name = "cmdTask",
  .stack_size = 128 * 8,
  .priority = (osPriority_t) osPriorityNormal,
};


void put_cmd(cmd_t *cmd)
{

  if (osMessageQueuePut(g_cmdMessageQueue, cmd, 0, osWaitForever) == osOK)
  {

  }
  else
  {

  }
}



void cmdTask(void *argument)
{
  cmd_t  cmd;
  osStatus status;

  while(1)
  {
      status = osMessageQueueGet(g_cmdMessageQueue, &cmd, NULL, osWaitForever);
      
      if(status ==osOK)
      {
        switch(cmd.cmd)
        {
          case HOST_CMD_READ_SYSTEM:
          break;
          case HOST_CMD_READ_CONFIG:
          break;
          case HOST_CMD_WRITE_CONFIG:
          break;
          case HOST_CMD_DOWNLOAD_FW:
          break;
          case HOST_CMD_UPDATE_FW:
          break;
        }
      }
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