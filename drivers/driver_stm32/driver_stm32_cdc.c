




#include <stdio.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "driver_stm32_cdc.h"
#include "semphr.h"
#include "util_memory.h"
#include "system_err.h"
#include "stm32_usb.h"
#include "pcb_define.h"

extern uint32_t calculate_txWaitTimeMs(uint32_t baud, uint16_t dataLen);


typedef struct stm32_cdc_cfg_s
{
  bool opened;
  uint8_t channel;// 채널 번호
  uint32_t baud;  // 설정된 통신속도
  int8_t errCode;// 드라이버 에러  상태 정보
  bool connected;
  UART_HandleTypeDef *handle;
  StreamBufferHandle_t cdc_stream;
  void *tx_sem;
  void *rx_sem;
  void *txcSem; // 전송 완료 알림 세마포어
}stm32_cdc_instance_t;


static stm32_cdc_instance_t cdc_inst;

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
    return 0;
  }

  if (cdc_inst.tx_sem == NULL)
  {
    tempSem = osSemaphoreNew(1, 1, NULL);
    if (tempSem)
    {
      cdc_inst.tx_sem = tempSem;
    }
  }

  if (cdc_inst.rx_sem == NULL)
  {
    tempSem = osSemaphoreNew(1, 1, NULL);
    if (tempSem)
    {
      cdc_inst.rx_sem = tempSem;
    }
  }


  if(cdc_inst.txcSem ==NULL)
  {
    tempSem = osSemaphoreNew(1, 0, NULL);
    if(tempSem)
      cdc_inst.txcSem = tempSem;
  }

  cdc_inst.cdc_stream = xStreamBufferCreate(100, 1);
  cdc_inst.opened = true;


    return 1;
}

int32_t stm32_cdc_send(int num,const uint8_t *pData,uint16_t dataLen)
{
  osStatus_t osStatus;
  int32_t retVal=dataLen;
  uint32_t waitTime;

  if(cdc_inst.connected==false || num < 0)
  {
    return -1;
  }
  
  if (cdc_inst.tx_sem)
  {
    osSemaphoreAcquire(cdc_inst.tx_sem, osWaitForever);
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

  if (cdc_inst.tx_sem)
  {
    osSemaphoreRelease(cdc_inst.tx_sem);
  }

  return retVal;

  
}
int32_t stm32_cdc_recv(int uart_num, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs)
{
  uint32_t start_tick;
  uint32_t elapsed_tick;
  uint32_t remaining_timeout;
  size_t bytes_available;
  size_t bytes_read;
  size_t cnt = 0;

  if (cdc_inst.connected == false || uart_num < 0)
  {
    return -1;
  }

  OS_PEND_SEM(cdc_inst.rx_sem, osWaitForever);

  // timeOutMs가 0인 경우: 논블로킹 모드
  if (timeOutMs == 0)
  {
    bytes_available = xStreamBufferBytesAvailable(cdc_inst.cdc_stream);

    if (bytes_available > 0)
    {
      size_t bytes_to_read = (bytes_available > buffSize) ? buffSize : bytes_available;
      bytes_read = xStreamBufferReceive(cdc_inst.cdc_stream,
                                        pBuff,
                                        bytes_to_read,
                                        0); // 대기시간 0
      cnt = bytes_read;
    }
    // 데이터가 없으면 cnt는 0으로 리턴

    OS_POST_SEM(cdc_inst.rx_sem);
    return cnt;
  }

  start_tick = osKernelGetTickCount();

  // timeOutMs가 0xFFFFFFFF인 경우: 무한 대기 모드
  if (timeOutMs == 0xFFFFFFFF)
  {
    while (cnt < buffSize)
    {
      bytes_available = xStreamBufferBytesAvailable(cdc_inst.cdc_stream);

      size_t bytes_to_read = buffSize - cnt;
      if (bytes_available > bytes_to_read)
      {
        bytes_available = bytes_to_read;
      }

      if (bytes_available == 0)
      {
        // 데이터가 없으면 최소 1바이트 수신까지 무한 대기
        bytes_read = xStreamBufferReceive(cdc_inst.cdc_stream,
                                          &pBuff[cnt],
                                          1,
                                          osWaitForever);
      }
      else
      {
        // 사용 가능한 데이터를 읽음
        bytes_read = xStreamBufferReceive(cdc_inst.cdc_stream,
                                          &pBuff[cnt],
                                          bytes_available,
                                          osWaitForever);
      }

      if (bytes_read > 0)
      {
        cnt += bytes_read;
      }
    }
  }
  // timeOutMs가 양수인 경우: 지정된 타임아웃 적용
  else
  {
    uint32_t timeout_tick = timeOutMs;

    while (cnt < buffSize)
    {
      elapsed_tick = osKernelGetTickCount() - start_tick;

      if (elapsed_tick >= timeout_tick)
      {
        break; // Timeout 발생
      }

      remaining_timeout = timeout_tick - elapsed_tick;

      bytes_available = xStreamBufferBytesAvailable(cdc_inst.cdc_stream);

      size_t bytes_to_read = buffSize - cnt;
      if (bytes_available > bytes_to_read)
      {
        bytes_available = bytes_to_read;
      }

      if (bytes_available == 0)
      {
        // 데이터가 없으면 최소 1바이트 수신 대기
        bytes_read = xStreamBufferReceive(cdc_inst.cdc_stream,
                                          &pBuff[cnt],
                                          1,
                                          remaining_timeout);
      }
      else
      {
        // 데이터를 읽음
        bytes_read = xStreamBufferReceive(cdc_inst.cdc_stream,
                                          &pBuff[cnt],
                                          bytes_available,
                                          remaining_timeout);
      }

      if (bytes_read > 0)
      {
        cnt += bytes_read;
      }
      else
      {
        // xStreamBufferReceive가 0을 리턴하면 타임아웃 발생
        break;
      }
    }
  }

  OS_POST_SEM(cdc_inst.rx_sem);

  return cnt;
}

int32_t stm32_cdc_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size,
                           uint32_t timeout1_ms, uint32_t timeout2_ms)
{
  int32_t cnt=0;
  if (cdc_inst.connected == false || num < 0)
  {
    return -1;
  }
  return cnt;
}





void cdc_tx_complete(void)
{
  if (cdc_inst.connected == false)
  {
    return ;
  }
  osSemaphoreRelease(cdc_inst.txcSem);
}

void put_cdc_rx(uint8_t *p_data,uint16_t dataLen)
{
  size_t xBytesSent;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  
  if(cdc_inst.cdc_stream)
  {
    xBytesSent = xStreamBufferSendFromISR(cdc_inst.cdc_stream, p_data, dataLen, &xHigherPriorityTaskWoken);
    /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
  (void)xBytesSent;
}



int32_t stm32_cdc_inject(int num, const uint8_t *pData, uint16_t dataLen)
{

  size_t xBytesSent=0;
  if (cdc_inst.connected == false ||  num< 0)
  {
    return -1;  
  }       

  if(cdc_inst.cdc_stream)
  {
  xBytesSent = xStreamBufferSend(cdc_inst.cdc_stream, pData, dataLen,
                                 pdMS_TO_TICKS(100));
  }
  return xBytesSent;

}


void stm32_cdc_flush_rx(void)
{
  
}


int32_t stm32_cdc_recv_crlf(int num, char *pBuff, uint16_t bSize, uint32_t tout_ms)
{
  uint8_t data;
  uint16_t cnt = 0;
  uint32_t startTime, startTick, stopTick, elapseTick;
  uint32_t timeout;
  uint32_t len;

  if (cdc_inst.connected == false <num <0)
  {
    return -1;
  }             
  
  startTime = osKernelGetTickCount();
  timeout = tout_ms;
 
  do
  {
    startTick = osKernelGetTickCount();
    len = stm32_cdc_recv(num, &data, 1, tout_ms);

    if (len)
    {
      pBuff[cnt++] = data;
      
      if((cnt==1)&&((data == '\r') || (data == '\n')))
      {
        cnt = 0;
        continue;
      }
          
         
      if ((data == '\r') || (data == '\n'))
      {
        pBuff[cnt - 1] = 0;
        return (cnt - 1); /* \r 또는 \n 를 제외한 문자열 길이 리턴*/
      }

      if (cnt == bSize)
      {
        return 0;
      }
    }

    stopTick = xTaskGetTickCount();
    elapseTick = stopTick - startTick;

    if ((tout_ms == 0) || ((stopTick - startTime) >= tout_ms))
    {
      break;
    }
    if (tout_ms != osWaitForever)
    {
      timeout = timeout - elapseTick;
    }
  } while (1);

  return 0;
}
