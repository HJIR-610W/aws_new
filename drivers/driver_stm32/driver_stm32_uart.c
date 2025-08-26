

#include "driver_stm32_uart.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "cmsis_os2.h"
//#include "semphr.h"
#include "os_user_def.h"
#include "stm32f4xx_hal.h"
#include "stream_buffer.h"
#include "system_err.h"
#include "util_memory.h"


typedef struct stm32_uart_cfg_s
{
  bool opened;
  int8_t errCode; // 드라이버 에러  상태 정보
  uint8_t parityIdx;
  uint32_t baud; // 설정된 통신속도
  uint8_t rxData;
  UART_HandleTypeDef handle;
  StreamBufferHandle_t xStreamBuffer;
  int buffser_size;
  DMA_HandleTypeDef dma_tx;
  DMA_HandleTypeDef dma_rx;
  void *tx_sem;
  void *rx_sem;
  void *txcSem; // 전송 완료 알림 세마포어
} uart_instance_t;

static uart_instance_t uart_inst[STM32_UART_MAX] = {[STM32_UART_0_CDMA] = {.handle.Instance = USART3,.buffser_size = 512},
                                                    [STM32_UART_1_SDI] = {.handle.Instance = USART6,.buffser_size = 50}};

DMA_HandleTypeDef *get_uart_txdma(int num)
{
   return &uart_inst[num].dma_tx;
 
}


static void stm32_uart_dma_init(int num)
{

  UART_HandleTypeDef *p_uart;
  DMA_HandleTypeDef *p_dma;

  p_uart = &uart_inst[num].handle;
  p_dma = &uart_inst[num].dma_tx;
  
  if (p_uart->Instance == USART3)
  {
    __HAL_RCC_DMA1_CLK_ENABLE();

    p_dma->Instance = DMA1_Stream3;
    p_dma->Init.Channel = DMA_CHANNEL_4;
    p_dma->Init.Direction = DMA_MEMORY_TO_PERIPH;
    p_dma->Init.PeriphInc = DMA_PINC_DISABLE;
    p_dma->Init.MemInc = DMA_MINC_ENABLE;
    p_dma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    p_dma->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    p_dma->Init.Mode = DMA_NORMAL;
    p_dma->Init.Priority = DMA_PRIORITY_LOW;
    p_dma->Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(p_dma) != HAL_OK)
    {
      ERROR_PRINTF("UART3 DMA");
    }
    __HAL_LINKDMA(p_uart, hdmatx, *p_dma);

    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

    HAL_NVIC_SetPriority(USART3_IRQn, 5, 1);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  }
  else if (p_uart->Instance == USART6)
  {
    __HAL_RCC_DMA2_CLK_ENABLE();

    p_dma->Instance = DMA2_Stream7;
    p_dma->Init.Channel = DMA_CHANNEL_5;
    p_dma->Init.Direction = DMA_MEMORY_TO_PERIPH;
    p_dma->Init.PeriphInc = DMA_PINC_DISABLE;
    p_dma->Init.MemInc = DMA_MINC_ENABLE;
    p_dma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    p_dma->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    p_dma->Init.Mode = DMA_NORMAL;
    p_dma->Init.Priority = DMA_PRIORITY_LOW;
    p_dma->Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(p_dma) != HAL_OK)
    {
      ERROR_PRINTF("UART6 DMA");
    }
    __HAL_LINKDMA(p_uart, hdmatx, *p_dma);

    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

    HAL_NVIC_SetPriority(USART6_IRQn, 5, 1);
    HAL_NVIC_EnableIRQ(USART6_IRQn);
  }
}

static void stm32_uart_hal_init(int num,uint32_t baud, uint8_t parity, uint8_t dataLen, uint8_t stop)
{
  UART_HandleTypeDef *p_uart;

  p_uart = &uart_inst[num].handle;

  p_uart->Instance = USART3;
  p_uart->Init.BaudRate = baud;
  if (dataLen == 0)
  {
    p_uart->Init.WordLength = UART_WORDLENGTH_8B;
  }
  else
  {
    p_uart->Init.WordLength = UART_WORDLENGTH_9B;
  }

  switch (stop)
  {
  case 2:
    p_uart->Init.StopBits = UART_STOPBITS_2;
    break;
  case 0:
  case 1:
    p_uart->Init.StopBits = UART_STOPBITS_1;
    break;
  }

  switch (parity)
  {
  case PARITY_EVEN:
    p_uart->Init.Parity = UART_PARITY_EVEN;
    break;
  case PARITY_ODD:
    p_uart->Init.Parity = UART_PARITY_ODD;
    break;
  case PARITY_NONE:
  default:
    p_uart->Init.Parity = UART_PARITY_NONE;
    break;
  }
  p_uart->Init.Mode = UART_MODE_TX_RX;
  p_uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  p_uart->Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(p_uart) != HAL_OK)
  {
    ERROR_PRINTF("UART3");
  }
}


int32_t stm32_uart_init(int num, void *opt)
{
  osSemaphoreId_t tempSem = NULL;
  uart_config_t *cfg = opt;

  if(uart_inst[num].opened == true)
  {
    return 1;
  }

  uart_inst[num].baud = cfg->baud;
  uart_inst[num].parityIdx = cfg->parityIdx;


  if (uart_inst[num].txcSem == NULL)
  {
    tempSem = osSemaphoreNew(1, 0, NULL);
    if (tempSem)
      uart_inst[num].txcSem = tempSem;
  }
  uart_inst[num].xStreamBuffer = xStreamBufferCreate(uart_inst[num].buffser_size, 1);
  OS_CREATE_BINARY_SEM(uart_inst[num].tx_sem);
  OS_CREATE_BINARY_SEM(uart_inst[num].rx_sem);

  stm32_uart_hal_init(num, cfg->baud, cfg->parityIdx, cfg->dataLen, cfg->stop_bit);
  stm32_uart_dma_init(num);
  HAL_UART_Receive_IT(&uart_inst[num].handle, (uint8_t *)&uart_inst[num], 1);


  uart_inst[num].opened = true;

  return 1;
}

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

void HAL_UART_IDLECallback(UART_HandleTypeDef *huart)
{ 
  //__asm("BKPT #0");
}

void USART3_IRQHandler(void)
{
  HAL_UART_IRQHandler(&uart_inst[STM32_UART_0_CDMA].handle);
}

void USART6_IRQHandler(void)
{
  HAL_UART_IRQHandler(&uart_inst[STM32_UART_1_SDI].handle);
}

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
    osSemaphoreRelease(uart_inst[STM32_UART_0_CDMA].txcSem);
  }
  else if (huart->Instance == USART6)
  {
    osSemaphoreRelease(uart_inst[STM32_UART_1_SDI].txcSem);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  size_t xBytesSent;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  for(int i=0;i<_countof(uart_inst);i++)
  {
    if(uart_inst[i].handle.Instance == huart->Instance)
    {
      xBytesSent = xStreamBufferSendFromISR(uart_inst[i].xStreamBuffer, &uart_inst[i].rxData, 1,
                                            &xHigherPriorityTaskWoken);
      if (!(xBytesSent > 0))
      {
        __asm("BKPT #0");//TODO:실행중 발생하면 usage fault 발생됨
      }
      /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      HAL_UART_Receive_IT(&uart_inst[i].handle, (uint8_t *)&uart_inst[i].rxData, 1);
      break;
    }
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
      //__asm("BKPT #0");
    }
    if (error & HAL_UART_ERROR_NE)
    {
      //__asm("BKPT #0");
    }
    if (error & HAL_UART_ERROR_FE)
    {
      //__asm("BKPT #0");
    }
    if (error & HAL_UART_ERROR_ORE)
    {
     // __asm("BKPT #0");
    }

    // 필요한 추가 오류 처리 작업 수행
  }
}

int32_t stm32_uart_recv(int uart_num, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs)
{
  uint32_t start_tick;
  uint32_t elapsed_tick;
  uint32_t remaining_timeout;
  size_t bytes_available;
  size_t bytes_read;
  size_t cnt = 0;

  OS_PEND_SEM(uart_inst[uart_num].rx_sem, osWaitForever);

  // timeOutMs가 0인 경우: 논블로킹 모드
  if (timeOutMs == 0)
  {
    bytes_available = xStreamBufferBytesAvailable(uart_inst[uart_num].xStreamBuffer);

    if (bytes_available > 0)
    {
      size_t bytes_to_read = (bytes_available > buffSize) ? buffSize : bytes_available;
      bytes_read = xStreamBufferReceive(uart_inst[uart_num].xStreamBuffer,
                                        pBuff,
                                        bytes_to_read,
                                        0); // 대기시간 0
      cnt = bytes_read;
    }
    // 데이터가 없으면 cnt는 0으로 리턴

    OS_POST_SEM(uart_inst[uart_num].rx_sem);
    return cnt;
  }

  start_tick = osKernelGetTickCount();

  // timeOutMs가 0xFFFFFFFF인 경우: 무한 대기 모드
  if (timeOutMs == 0xFFFFFFFF)
  {
    while (cnt < buffSize)
    {
      bytes_available = xStreamBufferBytesAvailable(uart_inst[uart_num].xStreamBuffer);

      size_t bytes_to_read = buffSize - cnt;
      if (bytes_available > bytes_to_read)
      {
        bytes_available = bytes_to_read;
      }

      if (bytes_available == 0)
      {
        // 데이터가 없으면 최소 1바이트 수신까지 무한 대기
        bytes_read = xStreamBufferReceive(uart_inst[uart_num].xStreamBuffer,
                                          &pBuff[cnt],
                                          1,
                                          osWaitForever);
      }
      else
      {
        // 사용 가능한 데이터를 읽음
        bytes_read = xStreamBufferReceive(uart_inst[uart_num].xStreamBuffer,
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
    uint32_t timeout_tick = pdMS_TO_TICKS(timeOutMs);

    while (cnt < buffSize)
    {
      elapsed_tick = osKernelGetTickCount() - start_tick;

      if (elapsed_tick >= timeout_tick)
      {
        break; // Timeout 발생
      }

      remaining_timeout = timeout_tick - elapsed_tick;

      bytes_available = xStreamBufferBytesAvailable(uart_inst[uart_num].xStreamBuffer);

      size_t bytes_to_read = buffSize - cnt;
      if (bytes_available > bytes_to_read)
      {
        bytes_available = bytes_to_read;
      }

      if (bytes_available == 0)
      {
        // 데이터가 없으면 최소 1바이트 수신 대기
        bytes_read = xStreamBufferReceive(uart_inst[uart_num].xStreamBuffer,
                                          &pBuff[cnt],
                                          1,
                                          remaining_timeout);
      }
      else
      {
        // 데이터를 읽음
        bytes_read = xStreamBufferReceive(uart_inst[uart_num].xStreamBuffer,
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

  OS_POST_SEM(uart_inst[uart_num].rx_sem);

  return cnt;
}

void stm32_uart_set(int num, eUART_SET_OPTION_t cmd, void *option)
{
  uart_config_t *cfg_baud;

  switch (cmd)
  {
    case eUART_SET_CONFIG:
      cfg_baud = (uart_config_t *)option;
      UART_SetBaudAndParity(&uart_inst[num].handle, cfg_baud->baud, 3); // parity는 변경 안함
      break;
  }
}

void stm32_uart_flush_rx(int num)
{
  uint8_t data;

  if(num <0)
  {
    return;
  }
  
  while (stm32_uart_recv(num, &data, 1, 0));
}

int32_t stm32_uart_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms,
                       uint32_t timeout2_ms)
{
  int32_t received = 0;
  uint8_t *p = buffer;

  if(num <0 )
  {
    return 0;
  }
  // Step 1: 첫 바이트 수신 (timeout1 사용)
  int32_t ret = stm32_uart_recv(num, p, 1, timeout1_ms);
  if (ret <= 0)
    return 0;  // 첫 바이트 수신 실패, 수신 없음

  received += ret;
  p += ret;

  // Step 2: 추가 바이트 수신 루프 (timeout2 사용)
  while (received < buffer_size)
  {
    ret = stm32_uart_recv(num, p, 1, timeout2_ms);
    if (ret <= 0)
      break;  // timeout2 안에 수신된 게 없으면 종료

    received += ret;
    p += ret;
  }

  return received;
}



void stm32_uart_get(int num, eUART_GET_OPTION_t cmd, void *option)
{
  uart_config_t *opt_cfg = option;

  if(num <0)
  {
    return ;
  }
  switch (cmd)
  {
    case eUART_GET_CONFIG:
      opt_cfg->baud = uart_inst[num].baud;
      opt_cfg->parityIdx = uart_inst[num].parityIdx;
      break;
  }
}


void stm32_uart_close(int num)
{
  (void)0;
}

int32_t stm32_uart_inject(int num, const uint8_t *pData, uint16_t dataLen)
{
  size_t xBytesSent;

  if(num <0)
  {
    return 0;
  }
  xBytesSent = xStreamBufferSend(uart_inst[num].xStreamBuffer, pData, dataLen,
                                 pdMS_TO_TICKS( 100 ));

  return xBytesSent;

}





int32_t stm32_uart_recv_crlf(int num, char *pBuff, uint16_t bSize, uint32_t tout_ms)
{
  uint8_t data;
  uint16_t cnt = 0;
  uint32_t startTime, startTick, stopTick, elapseTick;
  uint32_t timeout;
  uint32_t len;

  startTime = OS_GET_TICK();
  timeout = tout_ms;

  do
  {
    startTick = OS_GET_TICK();
    len = stm32_uart_recv(num, &data, 1, tout_ms);

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

    stopTick = OS_GET_TICK();
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

#define STM32_UART_TX_TIMEOUTMS 60000

uint32_t calculate_txWaitTimeMs(uint32_t baud, uint16_t dataLen)
{
  uint32_t waitTime;

  waitTime = (uint32_t)(((dataLen * 10) / (float)baud) * 1000) + 100; // 100정도 기본 delay 해줌

  return waitTime;
}
int32_t stm32_uart_send(int num, const uint8_t *pData, uint16_t dataLen)
{
  int32_t retVal = dataLen;
  uint32_t waitTime;
  HAL_StatusTypeDef status;
  osStatus_t osStatus;

  OS_PEND_SEM(uart_inst[num].tx_sem, osWaitForever);
  osSemaphoreAcquire(uart_inst[num].txcSem, 0); // 이전에 처리 못한건 제거
  waitTime = calculate_txWaitTimeMs(uart_inst[num].baud, dataLen);
  status = HAL_UART_Transmit_DMA(&uart_inst[num].handle, pData, dataLen);

  if (status == HAL_OK)
  {
    if (uart_inst[num].txcSem)
    {
      osStatus = osSemaphoreAcquire(uart_inst[num].txcSem, waitTime);
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
  OS_POST_SEM(uart_inst[num].tx_sem);
  return retVal;
}


//ll함수
/**
 * @brief os자원 없이 직접 읽기
 */
int32_t stm32_uart_recv_ll(int num, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs)
{
  uint32_t start_time = HAL_GetTick();
  uint8_t data = 0;
  int32_t len = 0;

  if (num < 0)
  {
    return 0;
  }

  while (1)
  {
    if (HAL_UART_Receive(&uart_inst[num].handle, &data, 1, 0) == HAL_OK)
    {
      pBuff[len++] = data;
    }
    if ((HAL_GetTick() - start_time) > timeOutMs)
    {
      break;
    }
  }

  return len;
}

//테스트 필요
void stm32_uart_set_config(int num, uart_config_t *config)
{
  UART_HandleTypeDef *p_uart;
  HAL_StatusTypeDef status;

  if (num < 0 || num >= STM32_UART_MAX || config == NULL)
  {
    return;
  }

  if (!uart_inst[num].opened)
  {
    return;
  }

  OS_PEND_SEM(uart_inst[num].tx_sem, osWaitForever);
  OS_PEND_SEM(uart_inst[num].rx_sem, osWaitForever);

  p_uart = &uart_inst[num].handle;

  HAL_UART_Abort(p_uart);

  status = HAL_UART_DeInit(p_uart);
  if (status != HAL_OK)
  {
    ERROR_PRINTF("UART DeInit failed");
    OS_POST_SEM(uart_inst[num].rx_sem);
    OS_POST_SEM(uart_inst[num].tx_sem);
    return;
  }

  uart_inst[num].baud = config->baud;
  uart_inst[num].parityIdx = config->parityIdx;

  p_uart->Init.BaudRate = config->baud;

  if (config->dataLen == UART_DATA_LEN_8)
  {
    p_uart->Init.WordLength = UART_WORDLENGTH_8B;
  }
  else
  {
    p_uart->Init.WordLength = UART_WORDLENGTH_9B;
  }

  switch (config->stop_bit)
  {
  case 2:
    p_uart->Init.StopBits = UART_STOPBITS_2;
    break;
  case 0:
  case 1:
  default:
    p_uart->Init.StopBits = UART_STOPBITS_1;
    break;
  }

  switch (config->parityIdx)
  {
  case PARITY_EVEN:
    p_uart->Init.Parity = UART_PARITY_EVEN;
    break;
  case PARITY_ODD:
    p_uart->Init.Parity = UART_PARITY_ODD;
    break;
  case PARITY_NONE:
  default:
    p_uart->Init.Parity = UART_PARITY_NONE;
    break;
  }

  p_uart->Init.Mode = UART_MODE_TX_RX;
  p_uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  p_uart->Init.OverSampling = UART_OVERSAMPLING_16;

  status = HAL_UART_Init(p_uart);
  if (status != HAL_OK)
  {
    ERROR_PRINTF("UART Init failed");
    OS_POST_SEM(uart_inst[num].rx_sem);
    OS_POST_SEM(uart_inst[num].tx_sem);
    return;
  }

  HAL_UART_Receive_IT(p_uart, (uint8_t *)&uart_inst[num].rxData, 1);

  OS_POST_SEM(uart_inst[num].rx_sem);
  OS_POST_SEM(uart_inst[num].tx_sem);
}