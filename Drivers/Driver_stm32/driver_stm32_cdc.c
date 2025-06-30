




#include <stdio.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "driver_stm32_cdc.h"
#include "usDelay.h"
#include "mcu_swo.h"
#include "semphr.h"
#include "util_memory.h"
#include "system_err.h"
#include "stm32_usb.h"
#include "pcb_define.h"

typedef struct stm32_cdc_cfg_s
{
  UART_HandleTypeDef *handle;
  void *txcSem;   // 전송 완료 알림 세마포어
  uint8_t channel;// 채널 번호
  uint32_t baud;  // 설정된 통신속도
  int8_t errCode;// 드라이버 에러  상태 정보
  bool connected;
}stm32_cdc_cfg_t;


StreamBufferHandle_t g_stm32_cdc_buff;

extern uint32_t calculate_txWaitTimeMs(uint32_t baud,uint16_t dataLen);



driver_t *stm32_cdc_open(int num,void *opt);
void stm32_cdc_close(driver_t *handle);

int32_t stm32_cdc_recv_opt(driver_t *drv, uint8_t *buffer, uint16_t buffer_size,
                           uint32_t timeout1_ms, uint32_t timeout2_ms);
void stm32_cdc_flush_rx(driver_t *handle);
int32_t stm32_cdc_send(driver_t *drv,const uint8_t *pData,uint16_t dataLen);
int32_t stm32_cdc_recv(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,uint32_t timeOutMs);
int32_t stm32_cdc_inject(driver_t *drv, const uint8_t *pData, uint16_t dataLen);

uart_api_t stm32_cdc_api={.close = stm32_cdc_close,
                           .send =stm32_cdc_send,
                           .recv =stm32_cdc_recv,
                           .flush_rx = stm32_cdc_flush_rx,
                           .recv_opt = stm32_cdc_recv_opt,
                            .inject =stm32_cdc_inject};


driver_t g_stm32_cdc;
stm32_cdc_cfg_t g_stm32_cdc_cfg;


extern uint8_t  g_usb_cdc_connected ;



void set_usb_cdc_connection(bool set)
{
  g_stm32_cdc_cfg.connected = set;
}
driver_t *stm32_cdc_open(int num,void *opt)
{
  osSemaphoreId_t tempSem=NULL;


  if(g_stm32_cdc.opened == true)
  {
    return &g_stm32_cdc;
  }


  
  usbTask_init();

  for(int i = 0 ;i< 5; i++)
  {
    if (g_stm32_cdc_cfg.connected)
    {
      break;
    }
    osDelay(100);
  }
  



  g_stm32_cdc.api = &stm32_cdc_api;
  g_stm32_cdc.cfg = &g_stm32_cdc_cfg;
    
    if(g_stm32_cdc.sem == NULL)
    {
      tempSem = osSemaphoreNew(1, 1, NULL);
      if(tempSem)
      {
        g_stm32_cdc.sem = tempSem;
      }
    }

    if(g_stm32_cdc_cfg.txcSem ==NULL)
    {
      tempSem = osSemaphoreNew(1, 0, NULL);
      if(tempSem)
      g_stm32_cdc_cfg.txcSem = tempSem;
    }

  switch (num)
  {
    case STM32_CDC:
    g_stm32_cdc.name = TOSTRING(STM32_CDC);
    g_stm32_cdc_buff =   xStreamBufferCreate(100, 1 ); 

    break;

  }
  
  g_stm32_cdc.opened = true;
  
  return &g_stm32_cdc;
}

int32_t stm32_cdc_send(driver_t *drv,const uint8_t *pData,uint16_t dataLen)
{
  stm32_cdc_cfg_t *cfg = (stm32_cdc_cfg_t *)drv->cfg;
  osStatus_t osStatus;
  int32_t retVal=dataLen;
  uint32_t waitTime;

  if(drv==NULL || drv->opened==false)
  {
    return 0;
  }

  if(cfg->connected==false)
  {
    return -1;
  }
  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }

  osSemaphoreAcquire(cfg->txcSem, 0);// 이전에 처리 못한건 제거 
  waitTime = calculate_txWaitTimeMs(cfg->baud,dataLen);
  retVal = cdc_send(pData,dataLen);

  if(cfg->txcSem)
  {
    osStatus = osSemaphoreAcquire(cfg->txcSem, waitTime);
     if(osStatus != osOK)
     {
      cfg->errCode = (int8_t)osStatus;
      
      retVal = -1;
     }
  }


  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

  return retVal;

  
}


int32_t stm32_cdc_recv(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,uint32_t timeOutMs)
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
        xBytesAvailable = xStreamBufferBytesAvailable( g_stm32_cdc_buff );

        if(remainBuffSize < xBytesAvailable)
        {
          xBytesAvailable = remainBuffSize;// 버퍼 수만큼만 읽기
        }

        starTick = xTaskGetTickCount();
        if( xBytesAvailable > 0 )
        {
            /* 데이터를 읽을 수 있다면, 데이터를 수신 */
            xBytesRead = xStreamBufferReceive( g_stm32_cdc_buff, ( void * ) &pBuff[cnt], xBytesAvailable, pdMS_TO_TICKS( timeout ) );
            
            if(xBytesRead >0)
            {
              cnt += xBytesRead;
              lastTick = xTaskGetTickCount();
            }
        }
        else
        {
            /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
            xBytesRead = xStreamBufferReceive( g_stm32_cdc_buff, ( void * ) &pBuff[cnt], 1, pdMS_TO_TICKS( timeout ) );
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


void stm32_cdc_flush_rx(driver_t *handle)
{
  
}



int32_t stm32_cdc_recv_1(driver_t *drv, uint8_t *pBuff, uint16_t buffSize,void *opt)
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
        xBytesAvailable = xStreamBufferBytesAvailable( g_stm32_cdc_buff );

        if(remainBuffSize < xBytesAvailable)
        {
          xBytesAvailable = remainBuffSize;// 버퍼 수만큼만 읽기
        }

        starTick = xTaskGetTickCount();
        if( xBytesAvailable > 0 )
        {
            /* 데이터를 읽을 수 있다면, 데이터를 수신 */
            xBytesRead = xStreamBufferReceive( g_stm32_cdc_buff, ( void * ) &pBuff[cnt], xBytesAvailable, pdMS_TO_TICKS( timeout ) );
            
            if(xBytesRead >0)
            {
              cnt += xBytesRead;
              lastTick = xTaskGetTickCount();
            }
        }
        else
        {
            /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
            xBytesRead = xStreamBufferReceive( g_stm32_cdc_buff, ( void * ) &pBuff[cnt], 1, pdMS_TO_TICKS( timeout ) );
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

int32_t stm32_cdc_recv_opt(driver_t *drv, uint8_t *buffer, uint16_t buffer_size,
                           uint32_t timeout1_ms, uint32_t timeout2_ms)
{
  int32_t cnt=0;

  return cnt;
}


void stm32_cdc_close(driver_t *handle)
{

}



void cdc_tx_complete(void)
{
  osSemaphoreRelease(g_stm32_cdc_cfg.txcSem);
}

void put_cdc_rx(uint8_t *p_data,uint16_t dataLen)
{
  size_t xBytesSent;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if(g_stm32_cdc_buff)
  {
  xBytesSent = xStreamBufferSendFromISR(g_stm32_cdc_buff,p_data, dataLen, &xHigherPriorityTaskWoken);
/* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
  (void)xBytesSent;
}



int32_t stm32_cdc_inject(driver_t *drv, const uint8_t *pData, uint16_t dataLen)
{

  size_t xBytesSent;



  xBytesSent = xStreamBufferSend(g_stm32_cdc_buff, pData, dataLen,
                                 pdMS_TO_TICKS( 100 ));

  return xBytesSent;

}
