

#include <stdio.h>

#include "stm32f4xx_hal.h"
#include "driver_stm32_uart.h"
#include "cmsis_os.h"
#include "mcu_delay.h"
#include "mcu_swo.h"
#include "semphr.h"
#include "utile.h"

UART_HandleTypeDef huart1 = {.Instance = USART1};
UART_HandleTypeDef huart3 = {.Instance = USART3};
UART_HandleTypeDef huart6 = {.Instance = USART6};

DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_usart6_tx;

DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart6_rx;

#define BUFFER_SIZE 1024+128
typedef struct uart_ring_s
{
  uint16_t size;
  uint16_t head;
  uint16_t tail;
  uint8_t buffer[BUFFER_SIZE];
  osSemaphoreId_t  sem;
}uart_ring_t;


typedef struct stm32_uart_cfg_s
{
  UART_HandleTypeDef *handle;
  void *txcSem;   //전송 완료 알림 세마포어
}stm32_uart_cfg_t;

uart_ring_t g_uart_ring[3];
driver_t g_stm32_uart[3];
stm32_uart_cfg_t g_stm32_uart_cfg[3]={{.handle = &huart1 },
                                      {.handle = &huart3},
                                      {.handle = &huart6}};


uint8_t g_uart_rx_dma_buffer[BUFFER_SIZE];



void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
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
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
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
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
  else if(uartHandle->Instance==USART6)
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
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN USART6_MspInit 1 */

  /* USER CODE END USART6_MspInit 1 */
  }
}




// USART1 초기화 함수
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;


    if (HAL_UART_Init(&huart1) != HAL_OK) {
        //    Error_Handler(__FILE__,__LINE__);
    }
    
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_ERR); 


    
}

// USART3 초기화 함수
static void MX_USART3_UART_Init(void) {
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart3) != HAL_OK) {
        //    Error_Handler(__FILE__,__LINE__);
    }
}

// USART4 초기화 함수
static void MX_USART6_UART_Init(void) {
    huart6.Instance = USART6;
    huart6.Init.BaudRate = 115200;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX_RX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart6) != HAL_OK) {
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

    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK) {
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
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK) {
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
   
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&g_uart_ring[0].buffer[0], 1);

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

    if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK) {
       //     Error_Handler(__FILE__,__LINE__);
    }
    __HAL_LINKDMA(&huart3, hdmatx, hdma_usart3_tx);

    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
    
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 1);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&g_uart_ring[1].buffer[0], 1);
}

static void MX_DMA_UART6_Init(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE();

    hdma_usart6_tx.Instance = DMA2_Stream6;
    hdma_usart6_tx.Init.Channel = DMA_CHANNEL_5;
    hdma_usart6_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart6_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_tx.Init.Mode = DMA_NORMAL;
    hdma_usart6_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart6_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_usart6_tx) != HAL_OK) {
       //     Error_Handler(__FILE__,__LINE__);
    }
    __HAL_LINKDMA(&huart6, hdmatx, hdma_usart6_tx);

    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

    HAL_NVIC_SetPriority(USART6_IRQn, 5, 1);
    HAL_NVIC_EnableIRQ(USART6_IRQn);

    HAL_UART_Receive_IT(&huart6, (uint8_t *)&g_uart_ring[2].buffer[0], 1);
    
}


void uart_ring_init(int num)
{
  g_uart_ring[num].tail = 0;
  g_uart_ring[num].head = 0;

  g_uart_ring[num].sem = osSemaphoreNew(BUFFER_SIZE, 0, NULL);
}



driver_t *stm32_uart_open(int num)
{
  osSemaphoreId_t tempSem=NULL;

  if(g_stm32_uart[num].opened == true)
  {
    return &g_stm32_uart[num];
  }

  switch (num)
  {
  case STM32_UART_1:
      g_stm32_uart[num].name = TOSTRING(STM32_UART_1);
    g_stm32_uart[num].num = num;
    g_stm32_uart[num].cfg = &g_stm32_uart_cfg[num];

    if(g_stm32_uart[num].sem == NULL)
    {
      tempSem = osSemaphoreNew(1, 1, NULL);
      if(tempSem)
      {
        g_stm32_uart[num].sem = tempSem;
      }
    }

    if(g_stm32_uart_cfg[num].txcSem ==NULL)
    {
      tempSem = osSemaphoreNew(1, 0, NULL);
      if(tempSem)
      g_stm32_uart_cfg[num].txcSem = tempSem;
    }

    uart_ring_init(num);
    MX_USART1_UART_Init();
    MX_DMA_UART1_Init();
    break;
  case STM32_UART_3:
    g_stm32_uart[num].name = TOSTRING(STM32_UART_3);
    g_stm32_uart[num].num = num;
    g_stm32_uart[num].cfg = &g_stm32_uart_cfg[num];

    tempSem = osSemaphoreNew(1, 1, NULL);
    if(tempSem)
    g_stm32_uart[num].sem = tempSem;


    tempSem = osSemaphoreNew(1, 0, NULL);
    if(tempSem)
    {
      g_stm32_uart_cfg[num].txcSem = tempSem;
    }

    uart_ring_init(num);
    MX_USART3_UART_Init();
    MX_DMA_UART3_Init();
  break;
  case STM32_UART_6:
    g_stm32_uart[num].name = TOSTRING(STM32_UART_6);
    g_stm32_uart[num].num = num;
    g_stm32_uart[num].cfg = &g_stm32_uart_cfg[num];
    
    if(g_stm32_uart[num].sem ==NULL)
    {
      tempSem = osSemaphoreNew(1, 1, NULL);
      if(tempSem)
      g_stm32_uart[num].sem = tempSem;
    }
    if(g_stm32_uart_cfg[num].txcSem == NULL)
    {
      tempSem = osSemaphoreNew(1, 0, NULL);
      if(tempSem)
      g_stm32_uart_cfg[num].txcSem = tempSem;

    }
    uart_ring_init(num);
    MX_USART6_UART_Init();
    MX_DMA_UART6_Init();

    break;
  }
  
g_stm32_uart[num].opened = true;
  return &g_stm32_uart[num];
}







#include "stm32f4xx_hal.h"

HAL_StatusTypeDef UART_SetBaudAndParity(UART_HandleTypeDef *huart, uint32_t baudrate, uint32_t parity)
{
    HAL_StatusTypeDef status;
    uint32_t setParity;

    switch(parity)
    {
      case 0:
      setParity=  UART_PARITY_NONE;
      break;
      case 1:
      setParity = UART_PARITY_ODD;
      break;
      case 2:
      setParity = UART_PARITY_EVEN;
      break;
      case 3:
      setParity = huart->Init.Parity;//변경 안함
      break;
    }
    // UART 통신을 일시적으로 중지
    status = HAL_UART_DeInit(huart);
    if (status != HAL_OK) {
        return status; // UART 해제 실패 시 에러 반환
    }

    // UART 설정 변경
    huart->Init.BaudRate = baudrate;     // 보드레이트 설정
  
    huart->Init.Parity   = setParity;       // 패리티 설정 (UART_PARITY_NONE, UART_PARITY_EVEN, UART_PARITY_ODD 중 선택)
    
    // UART 통신 재초기화
    status = HAL_UART_Init(huart);
    return status; // UART 초기화 성공 여부 반환
}




void stm32_uart_send(driver_t *drv,const uint8_t *pData,uint16_t dataLen)
{
  stm32_uart_cfg_t *cfg = (stm32_uart_cfg_t *)drv->cfg;
  HAL_StatusTypeDef status;
  
    if(drv==NULL || drv->opened==false)
    {
      return ;
    }

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }

  status = HAL_UART_Transmit_DMA(cfg->handle,pData, dataLen);

  if(status == HAL_OK)
  {
    if(cfg->txcSem)
    {
      osSemaphoreAcquire(cfg->txcSem, osWaitForever);
    }
  }
  else
  {
    printf("HAL_UART_Transmit_DMA:%d\r\n",status);
  }



  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

}

int RingBuffer_Read(uart_ring_t *rb, uint8_t *data,uint32_t timeOutMs)
{
    // 세마포어가 확보되면 데이터 읽기 (데이터가 없으면 대기)
    
    if (osSemaphoreAcquire(rb->sem, timeOutMs) == osOK)
    {
        // 링버퍼에서 데이터 읽기
        *data = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % BUFFER_SIZE;
        return 1;  // 읽기 성공
    }
    return 0;  // 읽기 실패
}

int RingBuffer_Read2(uart_ring_t *rb, uint8_t *data,uint16_t dataSize,uint32_t timeOutMs)
{
  uint32_t starTick = xTaskGetTickCount();
  uint32_t stopTick;
  uint32_t elapseTick;
  int cnt = 0;


  while(1)
  {
    starTick = xTaskGetTickCount();
    if (osSemaphoreAcquire(rb->sem, timeOutMs) == osOK)
    {
          stopTick = xTaskGetTickCount();
          elapseTick = stopTick-starTick;


          // 링버퍼에서 데이터 읽기
          data[cnt] = rb->buffer[rb->tail];
          rb->tail = (rb->tail + 1) % BUFFER_SIZE;
          cnt++;

          if(timeOutMs <= elapseTick || cnt==dataSize) 
          {
            break;
          }
          timeOutMs =timeOutMs-elapseTick; 
      }
      else
      {
        break;
      }
    }

    return cnt;  
}

uint16_t stm32_uart_recv(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,uint32_t timeOutMs)
{
  int cnt;

  cnt =RingBuffer_Read2(&g_uart_ring[drv->num],pBuff,buffSize,timeOutMs);

  return cnt;
       
}

uint16_t stm32_uart_recvFrame(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,uint32_t timeOutMs)
{
  int cnt;

  cnt =RingBuffer_Read2(&g_uart_ring[drv->num],pBuff,buffSize,timeOutMs);

  return cnt;
       
}

void stm32_uart_set(driver_t *drv,eUART_SET_CMD_t cmd,void *option)
{
  uart_baud_config_t *cfg_baud;;
  stm32_uart_cfg_t *cfg;

   cfg = (stm32_uart_cfg_t *)drv->cfg;
  switch(cmd)
  {
    case eUART_SET_CONFIG:
    cfg_baud = (uart_baud_config_t *)option;
    UART_SetBaudAndParity(cfg->handle,cfg_baud->baud,3);//parity는 변경 안함
    break;
  }
    
}

int stm32_uart_recv_byte(driver_t *drv,uint8_t *data,uint32_t timeOutms)
{
  if(RingBuffer_Read(&g_uart_ring[drv->num],data,timeOutms))
  {
    return 1;
  }

  return  0;
}

void stm32_uart_init(driver_t *drv)
{

    
}



void uart1_receive(UART_HandleTypeDef *huart)
{
// UART IDLE 라인 감지 인터럽트 발생 여부 확인
 // DMA의 현재 수신 위치를 읽어 링 버퍼에 데이터 저장
  uint16_t dma_current_pos = BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
  static uint16_t old_pos = 0;
  uint16_t head;

  head = g_uart_ring[0].head;

    if (dma_current_pos != old_pos) {
        // 새로운 데이터가 들어온 구간만큼 링 버퍼에 복사
        if (dma_current_pos > old_pos) {
            for (uint16_t i = old_pos; i < dma_current_pos; i++)
            {
                g_uart_ring[0].buffer[head] = g_uart_rx_dma_buffer[i];
                head = (head + 1) % BUFFER_SIZE;
                osSemaphoreRelease(g_uart_ring[0].sem);
            }
        } else {
            for (uint16_t i = old_pos; i < BUFFER_SIZE; i++)
            {
                 g_uart_ring[0].buffer[head] = g_uart_rx_dma_buffer[i];
                head = (head + 1) % BUFFER_SIZE;
                                osSemaphoreRelease(g_uart_ring[0].sem);
            }
            for (uint16_t i = 0; i < dma_current_pos; i++)
            {
              
                 g_uart_ring[0].buffer[head] = g_uart_rx_dma_buffer[i];
                head = (head + 1) % BUFFER_SIZE;
                                osSemaphoreRelease(g_uart_ring[0].sem);
            }
        }
        g_uart_ring[0].head = head;
    }
        old_pos = dma_current_pos;

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

      uart1_receive(huart);

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
    HAL_UART_IDLECallback(&huart1);         // IDLE 콜백 호출
  }
  else
  {
    HAL_UART_IRQHandler(&huart1);
  }
#else
    HAL_UART_IRQHandler(&huart1);
  
#endif
}

void USART3_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart3);
}

void USART6_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart6);
}



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
  uint16_t head=0;
  

  
    if (huart->Instance == USART1) 
    {
        head = g_uart_ring[0].head;
        g_uart_ring[0].head = (head + 1) % BUFFER_SIZE;

        osSemaphoreRelease(g_uart_ring[0].sem);
        head = g_uart_ring[0].head;
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&g_uart_ring[0].buffer[head], 1);
    }
    else if(huart->Instance == USART3) 
    {
        // 수신된 데이터를 링버퍼에 저장
        head = g_uart_ring[1].head;
        g_uart_ring[1].head = (head + 1) % BUFFER_SIZE;

        // 세마포어 증가 (데이터 개수 증가)
        osSemaphoreRelease(g_uart_ring[1].sem);

        // 다음 바이트 수신을 위한 인터럽트 활성화
        head = g_uart_ring[1].head;
        HAL_UART_Receive_IT(&huart3, (uint8_t *)&g_uart_ring[1].buffer[head], 1);
    }
    else if(huart->Instance == USART6) 
    {
        // 수신된 데이터를 링버퍼에 저장
        head = g_uart_ring[2].head;
        g_uart_ring[2].head = (head + 1) % BUFFER_SIZE;

        // 세마포어 증가 (데이터 개수 증가)
        osSemaphoreRelease(g_uart_ring[2].sem);

        // 다음 바이트 수신을 위한 인터럽트 활성화
        head = g_uart_ring[2].head;

        HAL_UART_Receive_IT(&huart6, (uint8_t *)&g_uart_ring[2].buffer[head], 1);
    }
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) 
{
      uint32_t isrflags   = READ_REG(huart->Instance->SR);// 이 시퀀스를 수행하면 에러가지워짐
      uint32_t data   = READ_REG(huart->Instance->DR);
    if (huart->Instance == USART1) 
    {

        // 오류 종류 확인
        uint32_t error = HAL_UART_GetError(huart);

        if (error & HAL_UART_ERROR_PE) {
           // printf("Parity Error\n");
        }
        if (error & HAL_UART_ERROR_NE) {
           // printf("Noise Error\n");
        }
        if (error & HAL_UART_ERROR_FE) {
           // printf("Framing Error\n");
        }
        if (error & HAL_UART_ERROR_ORE) {
          
           // printf("Overrun Error\n");
        }

        // 필요한 추가 오류 처리 작업 수행
    }
}
