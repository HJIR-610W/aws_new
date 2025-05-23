

#include "driver_stm32_uart.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "mcu_swo.h"
#include "semphr.h"
#include "stm32f4xx_hal.h"
#include "stream_buffer.h"
#include "system_err.h"
#include "usDelay.h"
#include "utile.h"

#define STM32_UART_0_BUFF_SIZE 512
#define STM32_UART_1_BUFF_SIZE 512
#define STM32_UART_2_BUFF_SIZE 512

#define BUFFER_SIZE 1024 + 128

uint8_t rxData[STM32_UART_MAX];

StreamBufferHandle_t g_stm32_xStreamBuffer[STM32_UART_MAX];

UART_HandleTypeDef huart1 = {.Instance = USART1};
UART_HandleTypeDef huart3 = {.Instance = USART3};
UART_HandleTypeDef huart6 = {.Instance = USART6};

DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_usart6_tx;

DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart6_rx;

typedef struct stm32_uart_cfg_s
{
  UART_HandleTypeDef *handle;
  void *txcSem;     // 전송 완료 알림 세마포어
  uint8_t channel;  // 채널 번호
  uint32_t baud;    // 설정된 통신속도
  uint8_t parityIdx;
  int8_t errCode;  // 드라이버 에러  상태 정보
} stm32_uart_cfg_t;

driver_t g_stm32_uart[STM32_UART_MAX];
stm32_uart_cfg_t g_stm32_uart_cfg[STM32_UART_MAX] = {
    {.handle = &huart1}, {.handle = &huart3}, {.handle = &huart6}};

uint8_t g_uart_rx_dma_buffer[BUFFER_SIZE];

void stm32_uart_flush_rx(driver_t *drv);
void stm32_uart_close(driver_t *handle);
void stm32_uart_set(driver_t *drv, uart_set_option_t cmd, void *option);
int32_t stm32_uart_send(driver_t *drv, const uint8_t *pData, uint16_t dataLen);
int32_t stm32_uart_recv(driver_t *drv, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs);
int32_t stm32_recv_opt2(driver_t *drv, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms,
                        uint32_t timeout2_ms);
void stm32_uart_get(driver_t *drv, uart_get_option_t cmd, void *option);

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART1)
  {
    /* USER CODE BEGIN USART1_MspInit 0 */

    /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN USART1_MspInit 1 */

    /* USER CODE END USART1_MspInit 1 */
  }
  else if (uartHandle->Instance == USART3)
  {
    /* USER CODE BEGIN USART3_MspInit 0 */

    /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USER CODE BEGIN USART3_MspInit 1 */

    /* USER CODE END USART3_MspInit 1 */
  }
  else if (uartHandle->Instance == USART6)
  {
    /* USER CODE BEGIN USART6_MspInit 0 */

    /* USER CODE END USART6_MspInit 0 */
    /* USART6 clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USER CODE BEGIN USART6_MspInit 1 */

    /* USER CODE END USART6_MspInit 1 */
  }
}

// USART1 초기화 함수
static void MX_USART1_UART_Init(uint32_t baud, uint8_t parity, uint8_t dataLen, uint8_t stop)
{
  uint32_t val;
  huart1.Instance = USART1;
  huart1.Init.BaudRate = baud;

  if (dataLen == 0)
  {
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
  }
  else
  {
    huart1.Init.WordLength = UART_WORDLENGTH_9B;
  }

  if (stop)
  {
    huart1.Init.StopBits = UART_STOPBITS_2;
  }
  else
  {
    huart1.Init.StopBits = UART_STOPBITS_1;
  }

  switch (parity)
  {
    case PARITY_EVEN:
      huart1.Init.Parity = UART_PARITY_EVEN;
      break;
    case PARITY_ODD:
      huart1.Init.Parity = UART_PARITY_ODD;
      break;
    case PARITY_NONE:
    default:
      huart1.Init.Parity = UART_PARITY_NONE;
      break;
  }

  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    //    Error_Handler(__FILE__,__LINE__);
  }

  __HAL_UART_ENABLE_IT(&huart1, UART_IT_ERR);
}

// USART3 초기화 함수
static void MX_USART3_UART_Init(uint32_t baud, uint8_t parity, uint8_t dataLen, uint8_t stop)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = baud;
  if (dataLen == 0)
  {
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
  }
  else
  {
    huart1.Init.WordLength = UART_WORDLENGTH_9B;
  }

  if (stop)
  {
    huart1.Init.StopBits = UART_STOPBITS_2;
  }
  else
  {
    huart1.Init.StopBits = UART_STOPBITS_1;
  }
  switch (parity)
  {
    case PARITY_EVEN:
      huart1.Init.Parity = UART_PARITY_EVEN;
      break;
    case PARITY_ODD:
      huart1.Init.Parity = UART_PARITY_ODD;
      break;
    case PARITY_NONE:
    default:
      huart1.Init.Parity = UART_PARITY_NONE;
      break;
  }
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    //    Error_Handler(__FILE__,__LINE__);
  }
}

// USART4 초기화 함수
static void MX_USART6_UART_Init(uint32_t baud, uint8_t parity, uint8_t dataLen, uint8_t stop)
{
  huart6.Instance = USART6;
  huart6.Init.BaudRate = baud;
  if (dataLen == 0)
  {
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
  }
  else
  {
    huart1.Init.WordLength = UART_WORDLENGTH_9B;
  }

  if (stop)
  {
    huart1.Init.StopBits = UART_STOPBITS_2;
  }
  else
  {
    huart1.Init.StopBits = UART_STOPBITS_1;
  }
  switch (parity)
  {
    case PARITY_EVEN:
      huart1.Init.Parity = UART_PARITY_EVEN;
      break;
    case PARITY_ODD:
      huart1.Init.Parity = UART_PARITY_ODD;
      break;
    case PARITY_NONE:
    default:
      huart1.Init.Parity = UART_PARITY_NONE;
      break;
  }
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    //     Error_Handler(__FILE__,__LINE__);
  }
}

#define UART1_RX_INT_USE 1
#define UART1_RX_DMA_USE 0

static void MX_DMA_UART1_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();

  hdma_usart1_tx.Instance = DMA2_Stream7;
  hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
  hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart1_tx.Init.Mode = DMA_NORMAL;
  hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
  hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

  if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
  {
    //     Error_Handler(__FILE__,__LINE__);
  }
  __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);

  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

  HAL_NVIC_SetPriority(USART1_IRQn, 6, 1);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

#if UART1_RX_DMA_USE

  // DMA 설정
  hdma_usart1_rx.Instance = DMA2_Stream2;  // DMA 스트림 설정 (USART1 RX용 스트림)
  hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
  hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;  // 순환 모드
  hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;

  // DMA 초기화
  if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
  {
    //    Error_Handler(__FILE__,__LINE__);
  }

  // DMA와 UART 링크
  __HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);

  // UART IDLE 라인 인터럽트 활성화
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

  // UART 수신을 DMA로 시작 (CIRCULAR 모드)
  HAL_UART_Receive_DMA(&huart1, &g_uart_rx_dma_buffer[0], BUFFER_SIZE);

  // HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  // HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

#else

  HAL_UART_Receive_IT(&huart1, (uint8_t *)&rxData[0], 1);

#endif
}

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
    //     Error_Handler(__FILE__,__LINE__);
  }
  __HAL_LINKDMA(&huart3, hdmatx, hdma_usart3_tx);

  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

  HAL_NVIC_SetPriority(USART3_IRQn, 5, 1);
  HAL_NVIC_EnableIRQ(USART3_IRQn);

  HAL_UART_Receive_IT(&huart3, (uint8_t *)&rxData[1], 1);
}

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
    //     Error_Handler(__FILE__,__LINE__);
  }
  __HAL_LINKDMA(&huart6, hdmatx, hdma_usart6_tx);

  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

  HAL_NVIC_SetPriority(USART6_IRQn, 5, 1);
  HAL_NVIC_EnableIRQ(USART6_IRQn);

  HAL_UART_Receive_IT(&huart6, (uint8_t *)&rxData[2], 1);
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
                             .recv_ll = stm32_uart_recv_ll};

driver_t *stm32_uart_open(int num, void *opt)
{
  osSemaphoreId_t tempSem = NULL;
  uart_config_t *cfg = opt;

  if (g_stm32_uart[num].opened == true)
  {
    return &g_stm32_uart[num];
  }

  g_stm32_uart_cfg[num].channel = num;
  g_stm32_uart_cfg[num].baud = cfg->baud;
  g_stm32_uart_cfg[num].parityIdx = cfg->parityIdx;
  g_stm32_uart[num].api = &stm32_uart_api;
  g_stm32_uart[num].cfg = &g_stm32_uart_cfg[num];

  if (g_stm32_uart[num].sem == NULL)
  {
    tempSem = osSemaphoreNew(1, 1, NULL);
    if (tempSem)
    {
      g_stm32_uart[num].sem = tempSem;
    }
  }

  if (g_stm32_uart_cfg[num].txcSem == NULL)
  {
    tempSem = osSemaphoreNew(1, 0, NULL);
    if (tempSem)
      g_stm32_uart_cfg[num].txcSem = tempSem;
  }

  switch (num)
  {
    case STM32_UART_0_DEBUG:
      g_stm32_uart[num].name = TOSTRING(STM32_UART_0_DEBUG);
      g_stm32_xStreamBuffer[num] = xStreamBufferCreate(STM32_UART_0_BUFF_SIZE, 1);
      MX_USART1_UART_Init(cfg->baud, cfg->parityIdx, cfg->dataLen, cfg->stop_bit);
      MX_DMA_UART1_Init();
      break;
    case STM32_UART_1_CDMA:
      g_stm32_uart[num].name = TOSTRING(STM32_UART_1_CDMA);
      g_stm32_xStreamBuffer[num] = xStreamBufferCreate(STM32_UART_2_BUFF_SIZE, 1);
      MX_USART3_UART_Init(cfg->baud, cfg->parityIdx, cfg->dataLen, cfg->stop_bit);
      MX_DMA_UART3_Init();
      break;
    case STM32_UART_2_SDI:
      g_stm32_uart[num].name = TOSTRING(STM32_UART_2_SDI);
      g_stm32_xStreamBuffer[num] = xStreamBufferCreate(STM32_UART_2_BUFF_SIZE, 1);
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
        cfg->errCode = (int8_t)osStatus;

        retVal = -1;
      }
    }
  }
  else
  {
    Error_Handler(__FILE__, __LINE__);
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

/*
최초이의 한번바이트가 수신된 상태에서 특정 시간동안 UART RX 라인이
High 있으면 idle 인터럽트 발생
uart 일반적으로 한번에 들어온다면 적용가능한 방법
그러나,바이트의 재수신 시간이 너무 짧다면 문제가될 요소는 있음
*/
uint16_t len;
void HAL_UART_IDLECallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    // DMA 수신을 멈추고, 수신된 데이터 길이 계산
    __HAL_DMA_DISABLE(&hdma_usart1_rx);

    // DMA 수신 재시작
    __HAL_DMA_SET_COUNTER(&hdma_usart3_rx, BUFFER_SIZE);
    __HAL_DMA_ENABLE(&hdma_usart3_rx);
  }
}

void USART1_IRQHandler(void)
{
#if UART1_RX_DMA_USE

  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);  // IDLE 플래그 클리어
    HAL_UART_IDLECallback(&huart1);      // IDLE 콜백 호출
  }
  else
  {
    HAL_UART_IRQHandler(&huart1);
  }
#else
  HAL_UART_IRQHandler(&huart1);

#endif
}

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
  if (huart->Instance == USART1)
  {
    osSemaphoreRelease(g_stm32_uart_cfg[0].txcSem);
  }
  else if (huart->Instance == USART3)
  {
    osSemaphoreRelease(g_stm32_uart_cfg[1].txcSem);
  }
  else if (huart->Instance == USART6)
  {
    osSemaphoreRelease(g_stm32_uart_cfg[2].txcSem);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  uint16_t head = 0;
  size_t xBytesSent;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (huart->Instance == USART1)
  {
    xBytesSent = xStreamBufferSendFromISR(g_stm32_xStreamBuffer[0], &rxData[0], 1,
                                          &xHigherPriorityTaskWoken);
    /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    HAL_UART_Receive_IT(&huart1, (uint8_t *)&rxData[0], 1);
  }
  else if (huart->Instance == USART3)
  {
    xBytesSent = xStreamBufferSendFromISR(g_stm32_xStreamBuffer[1], &rxData[1], 1,
                                          &xHigherPriorityTaskWoken);
    /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&rxData[1], 1);
  }
  else if (huart->Instance == USART6)
  {
    xBytesSent = xStreamBufferSendFromISR(g_stm32_xStreamBuffer[2], &rxData[2], 1,
                                          &xHigherPriorityTaskWoken);
    /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    HAL_UART_Receive_IT(&huart6, (uint8_t *)&rxData[2], 1);
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
      // printf("Parity Error\n");
    }
    if (error & HAL_UART_ERROR_NE)
    {
      // printf("Noise Error\n");
    }
    if (error & HAL_UART_ERROR_FE)
    {
      // printf("Framing Error\n");
    }
    if (error & HAL_UART_ERROR_ORE)
    {
      // printf("Overrun Error\n");
    }

    // 필요한 추가 오류 처리 작업 수행
  }
}

void stm32_uart_close(driver_t *handle) {}

int32_t stm32_uart_recv_1(driver_t *drv, uint8_t *pBuff, uint16_t buffSize, void *opt)
{
  uart_optTimeOut_t *optTimeOut = opt;
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

  timeout = optTimeOut->frameTimeOutMs;

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

    if (cnt)
    {
      timeout = optTimeOut->dataTimeOutMs;
    }
    else
    {
      timeout = timeout - elapseTick;
    }
  }
}

void stm32_uart_flush_rx(driver_t *handle)
{
  uint8_t data;
  while (stm32_uart_recv(handle, &data, 1, 0));
}

int32_t stm32_recv_opt(driver_t *drv, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms,
                       uint32_t timeout2_ms)
{
  typedef struct
  {
    uint32_t timeout1_ms;
    uint32_t timeout2_ms;
  } uart_recv_opt_t;

  uart_recv_opt_t opt;
  opt.timeout1_ms = timeout1_ms;
  opt.timeout2_ms = timeout2_ms;

  int32_t received = 0;
  uint8_t *p = buffer;

  // Step 1: 첫 바이트 수신 (timeout1 사용)
  int32_t ret = stm32_uart_recv(drv, p, 1, opt.timeout1_ms);
  if (ret <= 0)
    return 0;  // 첫 바이트 수신 실패, 수신 없음

  received += ret;
  p += ret;

  // Step 2: 추가 바이트 수신 루프 (timeout2 사용)
  while (received < buffer_size)
  {
    ret = stm32_uart_recv(drv, p, 1, opt.timeout2_ms);
    if (ret <= 0)
      break;  // timeout2 안에 수신된 게 없으면 종료

    received += ret;
    p += ret;
  }

  return received;
}

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