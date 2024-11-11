


#include <stdint.h>

#include "cmsis_os.h"
#include "driver_stm32_uart.h"

#include "task_cmd.h"
#include "cobs.h"

#define COBS_DELIMITER 0x00
osThreadId_t g_hostTaskHandle;

osMessageQueueId_t g_hostTxMessageQueue;

typedef struct hostMsg_s
{
  uint8_t data[512];
  uint16_t len;
}hostMsg_t;


const osThreadAttr_t hostRxTask_attributes = {
  .name = "hostRxTask",
  .stack_size = 2048/4,
  .priority = (osPriority_t) osPriorityNormal,
};
const osThreadAttr_t hostTxTask_attributes = {
  .name = "hostTxTask",
  .stack_size = 1024/4,
  .priority = (osPriority_t) osPriorityNormal,
};




driver_t * g_hostRs232 = NULL;

void put_hostMsg(hostMsg_t *hostMsg)
{

  if (osMessageQueuePut(g_hostTxMessageQueue, hostMsg, 0, osWaitForever) == osOK)
  {

  }
}
bool validate_frame(uint8_t *data,uint16_t len)
{
  uint8_t sum = 0;

  for(int i = 0 ;i< len-1;i++)
  {
    sum += data[i];
  }

  if(sum == data[len-1])
  {
    return true;
  }

  return false;

}

void hostRxTask(void *argument)
{
  uint8_t buff[512];
  uint8_t data;
  uint16_t frameCnt=0;
  uint16_t decoded_length;
  cmd_t cmd;
  osStatus status;
  
  while(1)
  {
    if (stm32_uart_recv_byte(g_hostRs232,&data,osWaitForever))
    {
        if (data == COBS_DELIMITER)
        {             // 패킷 종료 바이트 감지
            if (frameCnt > 0)
            {
                // COBS 디코딩 수행
                 decoded_length = cobs_decode(buff, frameCnt, cmd.data);
                if (decoded_length > 0)
                {
                  //cmd.source = 1;
                  cmd.len = decoded_length;
                  put_cmd(&cmd);
                }
                frameCnt = 0;  // 수신 버퍼 인덱스 초기화
            }
        }
        else
        {
            // 수신 버퍼에 데이터 저장
            if (frameCnt < sizeof(buff)) {
                buff[frameCnt++] = data;
            } else {
                // 버퍼 오버플로우 시 버퍼 초기화
                frameCnt = 0;
            }
        }
    }
  }
}



void hostTxTask(void *argument)
{
  osStatus status;
  hostMsg_t msg;
  while(1)
  {
      status = osMessageQueueGet(g_hostTxMessageQueue, &msg, NULL, osWaitForever);
      
      if(status== osOK)
      {
        stm32_uart_send(g_hostRs232,msg.data,msg.len);
      }
  }
}




void hostTask_init(void)
{
  g_hostRs232 = stm32_uart_open(STM32_UART_1);
  
  g_hostTxMessageQueue = osMessageQueueNew(2, sizeof(hostMsg_t), NULL);

  osThreadNew(hostTxTask, NULL, &hostTxTask_attributes);
  osThreadNew(hostRxTask, NULL, &hostRxTask_attributes);
}