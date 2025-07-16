




#include <stdio.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "driver_stm32_cdc.h"
#include "bsp_delay.h"
#include "bsp_swo.h"
#include "semphr.h"
#include "util_memory.h"
#include "system_err.h"
#include "stm32_usb.h"
#include "pcb_define.h"

extern uint32_t calculate_txWaitTimeMs(uint32_t baud, uint16_t dataLen);
extern uint8_t g_usb_cdc_connected;

typedef struct stm32_cdc_cfg_s
{
  UART_HandleTypeDef *handle;
  void *txcSem;   // 전송 완료 알림 세마포어
  uint8_t channel;// 채널 번호
  uint32_t baud;  // 설정된 통신속도
  int8_t errCode;// 드라이버 에러  상태 정보
  bool connected;
  StreamBufferHandle_t xStreamBuffer;
  bool opened;
  void *sem;
}stm32_cdc_instance_t;



stm32_cdc_instance_t cdc_inst;


void set_usb_cdc_connection(bool set)
{
  cdc_inst.connected = set;
}
int32_t stm32_cdc_init(int num,void *opt)
{
  osSemaphoreId_t tempSem=NULL;

  if (cdc_inst.opened == true)
  {
    return 1;
  }

  usbTask_init();

  for(int i = 0 ;i< 5; i++)
  {
    if (cdc_inst.connected)
    {
      break;
    }
    osDelay(100);
  }

  if (cdc_inst.connected==0)
  {
    return -1;
  }
    if (cdc_inst.sem == NULL)
    {
      tempSem = osSemaphoreNew(1, 1, NULL);
      if (tempSem)
      {
        cdc_inst.sem = tempSem;
      }
    }

    if(cdc_inst.txcSem ==NULL)
    {
      tempSem = osSemaphoreNew(1, 0, NULL);
      if(tempSem)
        cdc_inst.txcSem = tempSem;
    }

    cdc_inst.xStreamBuffer = xStreamBufferCreate(100, 1);
    cdc_inst.opened = true;


    
    return 1;
}

int32_t stm32_cdc_send(int num,const uint8_t *pData,uint16_t dataLen)
{
  osStatus_t osStatus;
  int32_t retVal=dataLen;
  uint32_t waitTime;

  if(cdc_inst.connected==false)
  {
    return -1;
  }
  if (cdc_inst.sem)
  {
    osSemaphoreAcquire(cdc_inst.sem, osWaitForever);
  }

  osSemaphoreAcquire(cdc_inst.txcSem, 0); // 이전에 처리 못한건 제거
  waitTime = calculate_txWaitTimeMs(cdc_inst.baud, dataLen);
  retVal = cdc_send(pData,dataLen);

  if (cdc_inst.txcSem)
  {
    osStatus = osSemaphoreAcquire(cdc_inst.txcSem, waitTime);
    if (osStatus != osOK)
    {
      cdc_inst.errCode = (int8_t)osStatus;

      retVal = -1;
     }
  }

  if (cdc_inst.sem)
  {
    osSemaphoreRelease(cdc_inst.sem);
  }

  return retVal;

  
}


int32_t stm32_cdc_recv(int num,uint8_t *pBuff,uint16_t buffSize,uint32_t timeOutMs)
{

    uint32_t starTick;
    uint32_t stopTick;
    uint32_t elapseTick;
    uint32_t timeout;
    uint32_t lastTick=0;
    size_t xBytesAvailable;
    size_t xBytesRead;
    size_t remainBuffSize = buffSize;
    size_t cnt = 0;



    timeout = timeOutMs;
    
    (void)lastTick;

    while(1)
    {
        /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
        xBytesAvailable = xStreamBufferBytesAvailable(cdc_inst.xStreamBuffer);

        if(remainBuffSize < xBytesAvailable)
        {
          xBytesAvailable = remainBuffSize;// 버퍼 수만큼만 읽기
        }

        starTick = xTaskGetTickCount();
        if( xBytesAvailable > 0 )
        {
            /* 데이터를 읽을 수 있다면, 데이터를 수신 */
            xBytesRead = xStreamBufferReceive(cdc_inst.xStreamBuffer, (void *)&pBuff[cnt], xBytesAvailable, pdMS_TO_TICKS(timeout));

            if(xBytesRead >0)
            {
              cnt += xBytesRead;
              lastTick = xTaskGetTickCount();
            }
        }
        else
        {
            /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
            xBytesRead = xStreamBufferReceive(cdc_inst.xStreamBuffer, (void *)&pBuff[cnt], 1, pdMS_TO_TICKS(timeout));
            if(xBytesRead ==1)
            {
              cnt += 1;
              lastTick = xTaskGetTickCount();
            }
        }
        stopTick = xTaskGetTickCount();
        elapseTick = stopTick-starTick;

        
  
        if(elapseTick >= timeout  || cnt >= buffSize)
        {
          return cnt;
        }
        remainBuffSize -= xBytesAvailable;
        timeout = timeout - elapseTick; 
    }
}






int32_t stm32_cdc_recv_1(int num, uint8_t *pBuff, uint16_t buffSize,void *opt)
{
  uart_optTimeOut_t *optTimeOut=opt;
  uint32_t starTick;
  uint32_t stopTick;
  uint32_t elapseTick;
  uint32_t timeout;
  uint32_t lastTick=0;
  size_t xBytesAvailable;
  size_t xBytesRead;
  size_t remainBuffSize = buffSize;
  size_t cnt = 0;




  timeout = optTimeOut->frameTimeOutMs;
    
  (void)lastTick;

  while(1)
  {
        /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
        xBytesAvailable = xStreamBufferBytesAvailable(cdc_inst.xStreamBuffer);

        if(remainBuffSize < xBytesAvailable)
        {
          xBytesAvailable = remainBuffSize;// 버퍼 수만큼만 읽기
        }

        starTick = xTaskGetTickCount();
        if( xBytesAvailable > 0 )
        {
            /* 데이터를 읽을 수 있다면, 데이터를 수신 */
            xBytesRead = xStreamBufferReceive(cdc_inst.xStreamBuffer, (void *)&pBuff[cnt], xBytesAvailable, pdMS_TO_TICKS(timeout));

            if(xBytesRead >0)
            {
              cnt += xBytesRead;
              lastTick = xTaskGetTickCount();
            }
        }
        else
        {
            /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
            xBytesRead = xStreamBufferReceive(cdc_inst.xStreamBuffer, (void *)&pBuff[cnt], 1, pdMS_TO_TICKS(timeout));
            if(xBytesRead ==1)
            {
              cnt += 1;
              lastTick = xTaskGetTickCount();
            }
        }


        stopTick = xTaskGetTickCount();
        elapseTick = stopTick-starTick;

        
  
        if(elapseTick >= timeout  || cnt >= buffSize)
        {
          return cnt;
        }
        remainBuffSize -= xBytesAvailable;


        if(cnt)
        {
          timeout = optTimeOut->dataTimeOutMs;
        }
        else
        {
        timeout = timeout - elapseTick; 
        }
    }
}

int32_t stm32_cdc_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size,
                           uint32_t timeout1_ms, uint32_t timeout2_ms)
{
  int32_t cnt=0;

  return cnt;
}





void cdc_tx_complete(void)
{
  osSemaphoreRelease(cdc_inst.txcSem);
}

void put_cdc_rx(uint8_t *p_data,uint16_t dataLen)
{
  size_t xBytesSent;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if(cdc_inst.xStreamBuffer)
  {
    xBytesSent = xStreamBufferSendFromISR(cdc_inst.xStreamBuffer, p_data, dataLen, &xHigherPriorityTaskWoken);
    /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
  (void)xBytesSent;
}



int32_t stm32_cdc_inject(int num, const uint8_t *pData, uint16_t dataLen)
{

  size_t xBytesSent;

  xBytesSent = xStreamBufferSend(cdc_inst.xStreamBuffer, pData, dataLen,
                                 pdMS_TO_TICKS(100));

  return xBytesSent;

}


void stm32_cdc_flush_rx(void)
{
  
}


