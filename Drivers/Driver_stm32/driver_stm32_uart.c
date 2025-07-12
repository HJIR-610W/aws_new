

#include "driver_stm32_uart.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "bsp_swo.h"
#include "semphr.h"
#include "stm32f4xx_hal.h"
#include "stream_buffer.h"
#include "system_err.h"
#include "bsp_delay.h"
#include "util_memory.h"

#define STM32_UART_CDMA_BUFF_SIZE 512
#define STM32_UART_SDI_BUFF_SIZE  50

typedef struct stm32_uart_cfg_s
{
  UART_HandleTypeDef *handle;
  void *txcSem;     // 전송 완료 알림 세마포어
  uint8_t channel;  // 채널 번호
  uint8_t parityIdx;
  int8_t errCode;  // 드라이버 에러  상태 정보
  uint8_t dma_use;
  uint32_t baud;  // 설정된 통신속도
} stm32_uart_cfg_t;


uint8_t rxData[STM32_UART_MAX];
StreamBufferHandle_t g_stm32_xStreamBuffer[STM32_UART_MAX];
UART_HandleTypeDef huart3 = {.Instance = USART3};
UART_HandleTypeDef huart6 = {.Instance = USART6};

DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_usart6_tx;

DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart6_rx;

driver_t g_stm32_uart[STM32_UART_MAX];
static stm32_uart_cfg_t s_stm32_uart_cfg[STM32_UART_MAX] = {{.handle = &huart3}, {.handle = &huart6}};



void stm32_uart_flush_rx(driver_t *drv);
void stm32_uart_close(driver_t *handle);
void stm32_uart_set(driver_t *drv, uart_set_option_t cmd, void *option);
int32_t stm32_uart_send(driver_t *drv, const uint8_t *pData, uint16_t dataLen);
int32_t stm32_uart_recv(driver_t *drv, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs);
int32_t stm32_recv_opt2(driver_t *drv, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms,
                        uint32_t timeout2_ms);
void stm32_uart_get(driver_t *drv, uart_get_option_t cmd, void *option);
int32_t stm32_uart_inject(driver_t *drv, const uint8_t *pData, uint16_t dataLen);

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART1)
  {

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  }
  else if (uartHandle->Instance == USART3)
  {
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
  else if (uartHandle->Instance == USART6)
  {
    __HAL_RCC_USART6_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }
}


// USART3 초기화 함수
static void MX_USART3_UART_Init(uint32_t baud, uint8_t parity, uint8_t dataLen, uint8_t stop)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = baud;
  if (dataLen == 0)
  {
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
  }
  else
  {
    huart3.Init.WordLength = UART_WORDLENGTH_9B;
  }

  switch(stop)
  {
    case 2:
      huart3.Init.StopBits = UART_STOPBITS_2;
      break;
    case 0:
    case 1:
      huart3.Init.StopBits = UART_STOPBITS_1;
      break;
  }

  switch (parity)
  {
    case PARITY_EVEN:
      huart3.Init.Parity = UART_PARITY_EVEN;
      break;
    case PARITY_ODD:
      huart3.Init.Parity = UART_PARITY_ODD;
      break;
    case PARITY_NONE:
    default:
      huart3.Init.Parity = UART_PARITY_NONE;
      break;
  }
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    ERROR_PRINTF("UART3");
  }
}

// USART4 초기화 함수
static void MX_USART6_UART_Init(uint32_t baud, uint8_t parity, uint8_t dataLen, uint8_t stop)
{
  huart6.Instance = USART6;
  huart6.Init.BaudRate = baud;
  if (dataLen == 0)
  {
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
  }
  else
  {
    huart6.Init.WordLength = UART_WORDLENGTH_9B;
  }


    switch (stop)
    {
      case 2:
        huart6.Init.StopBits = UART_STOPBITS_2;
        break;
      case 0:
      case 1:
        huart6.Init.StopBits = UART_STOPBITS_1;
        break;
    }


  switch (parity)
  {
    case PARITY_EVEN:
      huart6.Init.Parity = UART_PARITY_EVEN;
      break;
    case PARITY_ODD:
      huart6.Init.Parity = UART_PARITY_ODD;
      break;
    case PARITY_NONE:
    default:
      huart6.Init.Parity = UART_PARITY_NONE;
      break;
  }
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    ERROR_PRINTF("UART6");
  }
}

#define UART1_RX_INT_USE 1
#define UART1_RX_DMA_USE 0
static void MX_DMA_UART3_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  hdma_usart3_tx.Instance = DMA1_Stream3;
  hdma_usart3_tx.Init.Channel = DMA_CHANNEL_4;
  hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart3_tx.Init.Mode = DMA_NORMAL;
  hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
  hdma_usart3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

  if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
  {
    ERROR_PRINTF("UART3 DMA");
  }
  __HAL_LINKDMA(&huart3, hdmatx, hdma_usart3_tx);

  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

  HAL_NVIC_SetPriority(USART3_IRQn, 5, 1);
  HAL_NVIC_EnableIRQ(USART3_IRQn);

  HAL_UART_Receive_IT(&huart3, (uint8_t *)&rxData[STM32_UART_0_CDMA], 1);
}

//
static void MX_DMA_UART6_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();

  hdma_usart6_tx.Instance = DMA2_Stream7;
  hdma_usart6_tx.Init.Channel = DMA_CHANNEL_5;
  hdma_usart6_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_usart6_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart6_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart6_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart6_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart6_tx.Init.Mode = DMA_NORMAL;
  hdma_usart6_tx.Init.Priority = DMA_PRIORITY_LOW;
  hdma_usart6_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

  if (HAL_DMA_Init(&hdma_usart6_tx) != HAL_OK)
  {
    ERROR_PRINTF("UART6 DMA");
  }
  __HAL_LINKDMA(&huart6, hdmatx, hdma_usart6_tx);

  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

  HAL_NVIC_SetPriority(USART6_IRQn, 5, 1);
  HAL_NVIC_EnableIRQ(USART6_IRQn);

  HAL_UART_Receive_IT(&huart6, (uint8_t *)&rxData[STM32_UART_0_CDMA], 1);
}

driver_t *stm32_uart_open(int num, void *opt);
void stm32_uart_close(driver_t *handle);
void stm32_uart_flush_rx(driver_t *handle);
int32_t stm32_recv_opt(driver_t *drv, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms,
                       uint32_t timeout2_ms);
int32_t stm32_uart_recv_ll(driver_t *drv, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs);

uart_api_t stm32_uart_api = {.close = stm32_uart_close,
                             .send = stm32_uart_send,
                             .recv = stm32_uart_recv,
                             .flush_rx = stm32_uart_flush_rx,
                             .recv_opt = stm32_recv_opt,
                             .get = stm32_uart_get,
                             .recv_ll = stm32_uart_recv_ll,
                             .inject = stm32_uart_inject};

driver_t *stm32_uart_open(int num, void *opt)
{
  osSemaphoreId_t tempSem = NULL;
  uart_config_t *cfg = opt;

  if (g_stm32_uart[num].opened == true)
  {
    return &g_stm32_uart[num];
  }

  s_stm32_uart_cfg[num].channel = num;
  s_stm32_uart_cfg[num].baud = cfg->baud;
  s_stm32_uart_cfg[num].parityIdx = cfg->parityIdx;
  g_stm32_uart[num].api = &stm32_uart_api;
  g_stm32_uart[num].cfg = &s_stm32_uart_cfg[num];

  if (g_stm32_uart[num].sem == NULL)
  {
    tempSem = osSemaphoreNew(1, 1, NULL);
    if (tempSem)
    {
      g_stm32_uart[num].sem = tempSem;
    }
  }

  if (s_stm32_uart_cfg[num].txcSem == NULL)
  {
    tempSem = osSemaphoreNew(1, 0, NULL);
    if (tempSem)
      s_stm32_uart_cfg[num].txcSem = tempSem;
  }

  switch (num)
  {
    case STM32_UART_0_CDMA:
      s_stm32_uart_cfg[STM32_UART_0_CDMA].handle = &huart3;
      s_stm32_uart_cfg[num].dma_use = 1;
      g_stm32_uart[num].name = TOSTRING(STM32_UART_0_CDMA);
      g_stm32_xStreamBuffer[num] = xStreamBufferCreate(STM32_UART_CDMA_BUFF_SIZE, 1);
      MX_USART3_UART_Init(cfg->baud, cfg->parityIdx, cfg->dataLen, cfg->stop_bit);
      MX_DMA_UART3_Init();
      break;
    case STM32_UART_1_SDI:
      s_stm32_uart_cfg[num].handle = &huart6;
      s_stm32_uart_cfg[num].dma_use = 1;
      g_stm32_uart[num].name = TOSTRING(STM32_UART_1_SDI);
      g_stm32_xStreamBuffer[num] = xStreamBufferCreate(STM32_UART_SDI_BUFF_SIZE, 1);
      MX_USART6_UART_Init(cfg->baud, cfg->parityIdx, cfg->dataLen, cfg->stop_bit);
      MX_DMA_UART6_Init();
      break;
  }

  g_stm32_uart[num].opened = true;

  return &g_stm32_uart[num];
}

HAL_StatusTypeDef UART_SetBaudAndParity(UART_HandleTypeDef *huart, uint32_t baudrate,
                                        uint32_t parity)
{
  HAL_StatusTypeDef status;
  uint32_t setParity;

  switch (parity)
  {
    case 0:
      setParity = UART_PARITY_NONE;
      break;
    case 1:
      setParity = UART_PARITY_ODD;
      break;
    case 2:
      setParity = UART_PARITY_EVEN;
      break;
    case 3:
      setParity = huart->Init.Parity;  // 변경 안함
      break;
  }
  // UART 통신을 일시적으로 중지
  status = HAL_UART_DeInit(huart);
  if (status != HAL_OK)
  {
    return status;  // UART 해제 실패 시 에러 반환
  }

  // UART 설정 변경
  huart->Init.BaudRate = baudrate;  // 보드레이트 설정

  huart->Init.Parity = setParity;  // 패리티 설정 (UART_PARITY_NONE,
                                   // UART_PARITY_EVEN, UART_PARITY_ODD 중 선택)

  // UART 통신 재초기화
  status = HAL_UART_Init(huart);
  return status;  // UART 초기화 성공 여부 반환
}





/*
최초이의 한번바이트가 수신된 상태에서 특정 시간동안 UART RX 라인이
High 있으면 idle 인터럽트 발생
uart 일반적으로 한번에 들어온다면 적용가능한 방법
그러나,바이트의 재수신 시간이 너무 짧다면 문제가될 요소는 있음
*/

void HAL_UART_IDLECallback(UART_HandleTypeDef *huart) { __asm("BKPT #0"); }

void USART3_IRQHandler(void) { HAL_UART_IRQHandler(&huart3); }

void USART6_IRQHandler(void) { HAL_UART_IRQHandler(&huart6); }

/*
UART TX 완료 처리
DMA 사용해서 데이터 전송시 마지막 데이터가 전송되고 나면
전송완료 인터럽트 발생
DMA 전송완료가 아닌 UART TX 전송 완료로 전송완료를 처리해야함
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
if (huart->Instance == USART3)
  {
    osSemaphoreRelease(s_stm32_uart_cfg[STM32_UART_0_CDMA].txcSem);
  }
  else if (huart->Instance == USART6)
  {
    osSemaphoreRelease(s_stm32_uart_cfg[STM32_UART_1_SDI].txcSem);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  size_t xBytesSent;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

if (huart->Instance == USART3)
  {
    xBytesSent = xStreamBufferSendFromISR(g_stm32_xStreamBuffer[STM32_UART_0_CDMA], &rxData[STM32_UART_0_CDMA], 1,
                                          &xHigherPriorityTaskWoken);
    if (!(xBytesSent > 0))
    {
      __asm("BKPT #0");
    }
    /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&rxData[STM32_UART_0_CDMA], 1);
  }
  else if (huart->Instance == USART6)
  {
    xBytesSent = xStreamBufferSendFromISR(g_stm32_xStreamBuffer[STM32_UART_1_SDI],
                                          &rxData[STM32_UART_1_SDI], 1, &xHigherPriorityTaskWoken);
    if (!(xBytesSent > 0))
    {
      __asm("BKPT #0");
    }
    /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    HAL_UART_Receive_IT(&huart6, (uint8_t *)&rxData[STM32_UART_1_SDI], 1);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  uint32_t isrflags = READ_REG(huart->Instance->SR);  // 이 시퀀스를 수행하면 에러가지워짐
  uint32_t data = READ_REG(huart->Instance->DR);
  if (huart->Instance == USART1)
  {
    // 오류 종류 확인
    uint32_t error = HAL_UART_GetError(huart);

    if (error & HAL_UART_ERROR_PE)
    {
      __asm("BKPT #0");
    }
    if (error & HAL_UART_ERROR_NE)
    {
      __asm("BKPT #0");
    }
    if (error & HAL_UART_ERROR_FE)
    {
      __asm("BKPT #0");
    }
    if (error & HAL_UART_ERROR_ORE)
    {
      __asm("BKPT #0");
    }

    // 필요한 추가 오류 처리 작업 수행
  }
}

#define STM32_UART_TX_TIMEOUTMS 60000

uint32_t calculate_txWaitTimeMs(uint32_t baud, uint16_t dataLen)
{
  uint32_t waitTime;

  waitTime = (uint32_t)(((dataLen * 10) / (float)baud) * 1000) + 100;  // 100정도 기본 delay 해줌

  return waitTime;
}
int32_t stm32_uart_send(driver_t *drv, const uint8_t *pData, uint16_t dataLen)
{
  stm32_uart_cfg_t *cfg = (stm32_uart_cfg_t *)drv->cfg;
  HAL_StatusTypeDef status;
  osStatus_t osStatus;
  int32_t retVal = dataLen;
  uint32_t waitTime;

  if (drv == NULL || drv->opened == false)
  {
    return 0;
  }

  if (drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }

  osSemaphoreAcquire(cfg->txcSem, 0);  // 이전에 처리 못한건 제거
  waitTime = calculate_txWaitTimeMs(cfg->baud, dataLen);
  status = HAL_UART_Transmit_DMA(cfg->handle, pData, dataLen);

  if (status == HAL_OK)
  {
    if (cfg->txcSem)
    {
      osStatus = osSemaphoreAcquire(cfg->txcSem, waitTime);
      if (osStatus != osOK)
      {
        ERROR_PRINTF("uart %d", osStatus);

        retVal = -1;
      }
    }
  }
  else
  {
    ERROR_PRINTF("uart");
  }

  if (drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

  return retVal;
}

int32_t stm32_uart_recv(driver_t *drv, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs)
{
  uint32_t starTick;
  uint32_t stopTick;
  uint32_t elapseTick;
  uint32_t timeout;
  uint32_t lastTick = 0;
  size_t xBytesAvailable;
  size_t xBytesRead;
  size_t remainBuffSize = buffSize;
  size_t cnt = 0;

  stm32_uart_cfg_t *cfg = drv->cfg;
  uint8_t channel = cfg->channel;
  timeout = timeOutMs;

  (void)lastTick;

  while (1)
  {
    /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
    xBytesAvailable = xStreamBufferBytesAvailable(g_stm32_xStreamBuffer[channel]);

    if (remainBuffSize < xBytesAvailable)
    {
      xBytesAvailable = remainBuffSize;  // 버퍼 수만큼만 읽기
    }

    starTick = xTaskGetTickCount();
    if (xBytesAvailable > 0)
    {
      /* 데이터를 읽을 수 있다면, 데이터를 수신 */
      xBytesRead = xStreamBufferReceive(g_stm32_xStreamBuffer[channel], (void *)&pBuff[cnt],
                                        xBytesAvailable, pdMS_TO_TICKS(timeout));

      if (xBytesRead > 0)
      {
        cnt += xBytesRead;
        lastTick = xTaskGetTickCount();
      }
    }
    else
    {
      /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
      xBytesRead = xStreamBufferReceive(g_stm32_xStreamBuffer[channel], (void *)&pBuff[cnt], 1,
                                        pdMS_TO_TICKS(timeout));
      if (xBytesRead == 1)
      {
        cnt += 1;
        lastTick = xTaskGetTickCount();
      }
    }
    stopTick = xTaskGetTickCount();
    elapseTick = stopTick - starTick;

    if (elapseTick >= timeout || cnt >= buffSize)
    {
      return cnt;
    }
    remainBuffSize -= xBytesAvailable;
    timeout = timeout - elapseTick;
  }
}

void stm32_uart_set(driver_t *drv, uart_set_option_t cmd, void *option)
{
  uart_config_t *cfg_baud;
  ;
  stm32_uart_cfg_t *cfg;

  cfg = (stm32_uart_cfg_t *)drv->cfg;
  switch (cmd)
  {
    case eUART_SET_CONFIG:
      cfg_baud = (uart_config_t *)option;
      UART_SetBaudAndParity(cfg->handle, cfg_baud->baud,
                            3);  // parity는 변경 안함
      break;
  }
}

void stm32_uart_flush_rx(driver_t *drv)
{
  uint8_t data;
  while (stm32_uart_recv(drv, &data, 1, 0));
}

int32_t stm32_recv_opt(driver_t *drv, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms,
                       uint32_t timeout2_ms)
{
  int32_t received = 0;
  uint8_t *p = buffer;

  // Step 1: 첫 바이트 수신 (timeout1 사용)
  int32_t ret = stm32_uart_recv(drv, p, 1, timeout1_ms);
  if (ret <= 0)
    return 0;  // 첫 바이트 수신 실패, 수신 없음

  received += ret;
  p += ret;

  // Step 2: 추가 바이트 수신 루프 (timeout2 사용)
  while (received < buffer_size)
  {
    ret = stm32_uart_recv(drv, p, 1, timeout2_ms);
    if (ret <= 0)
      break;  // timeout2 안에 수신된 게 없으면 종료

    received += ret;
    p += ret;
  }

  return received;
}


/**
 * @brief os자원 없이 직접 읽기 
 */
int32_t stm32_uart_recv_ll(driver_t *drv, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs)
{
  stm32_uart_cfg_t *p_cfg = drv->cfg;
  uint32_t start_time  = HAL_GetTick();
  uint8_t data=0;
  int32_t len=0;

  while(1)
  {
    if(HAL_UART_Receive(p_cfg->handle, &data, 1, 0)==HAL_OK)
    {
      pBuff[len++] = data;
    }

    if((HAL_GetTick()-start_time)>timeOutMs)
    {
      break;
    }
  }


  return len;
}
void stm32_uart_get(driver_t *drv, uart_get_option_t cmd, void *option)
{
  uart_config_t *opt_cfg = option;
  stm32_uart_cfg_t *cfg;

  cfg = (stm32_uart_cfg_t *)drv->cfg;
  switch (cmd)
  {
    case UART_GET_CONFIG:
      opt_cfg->baud = cfg->baud;
      opt_cfg->parityIdx = cfg->parityIdx;
      break;
  }
}


void stm32_uart_close(driver_t *handle)
{
  (void)0;
}

int32_t stm32_uart_inject(driver_t *drv, const uint8_t *pData, uint16_t dataLen)
{

  size_t xBytesSent;
  stm32_uart_cfg_t *cfg = drv->cfg;


  xBytesSent = xStreamBufferSend(g_stm32_xStreamBuffer[cfg->channel], pData, dataLen,
                                 pdMS_TO_TICKS( 100 ));

  return xBytesSent;

}