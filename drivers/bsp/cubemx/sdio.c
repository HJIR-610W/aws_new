#include "sdio.h"
#include "pcb_define.h"
#include "system_err.h"

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;


uint32_t getSDIOClockFrequency(void)
{
    uint32_t systemClock = HAL_RCC_GetSysClockFreq(); // 시스템 클럭 가져오기
    uint32_t ahbPrescaler = (RCC->CFGR & RCC_CFGR_HPRE) >> 4; // AHB 프리스케일러 추출

    // AHB 프리스케일러 값에 따라 나눗셈 설정
    uint32_t ahbDivider;
    if (ahbPrescaler < 8) {
        ahbDivider = 1; // 프리스케일러 값이 0b0000(분주 없음)일 때
    } else {
        ahbDivider = 2 << (ahbPrescaler - 8); // 프리스케일러 값이 0b1000(분주 시작)부터
    }

    uint32_t ahbClock = systemClock / ahbDivider; // AHB 클럭 계산
    return ahbClock; // SDIO의 메인 클럭 속도 반환
}

uint32_t calculateSDIOClockDiv(uint32_t hclk, uint32_t pclk2, uint32_t desired_sdio_clk)
{
  uint32_t clkdiv;
  uint32_t actual_clk;

  // SDIO 클럭 최대치 보정
  if (desired_sdio_clk > 48000000) {
    desired_sdio_clk = 48000000; // USB 사용 고려 시 안전한 값
  }

  // CLKDIV 계산: SDIO_CK = HCLK / (CLKDIV + 2)
  clkdiv = (hclk / desired_sdio_clk) - 2;

  if ((int32_t)clkdiv < 0) {
    clkdiv = 0;
  } else if (clkdiv > 0xFF) {
    clkdiv = 0xFF;
  }

  // 실제 SDIO 클럭
  actual_clk = hclk / (clkdiv + 2);

#if 0
  // PCLK2 제약 조건 확인
  if (pclk2 < (3 * actual_clk) / 8) {
    printf("Warning: PCLK2(%lu Hz) too low for SDIO clock %lu Hz\n",
           pclk2, actual_clk);
  }

  printf("HCLK=%lu Hz, Desired=%lu Hz, Actual SDIO=%lu Hz, CLKDIV=%lu, PCLK2=%lu Hz\n",
         hclk, desired_sdio_clk, actual_clk, clkdiv, pclk2);
#endif
  return clkdiv;
}

#define SDIO_CLOCK_FREQ 21000000

void MX_SDIO_SD_Init(void)
{
  uint32_t g_sdioMainClk;

  g_sdioMainClk = getSDIOClockFrequency();//168MHz

  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 1;//14MHz calculateSDIOClockDiv(168000000,84000000, SDIO_CLOCK_FREQ);
}


void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(sdHandle->Instance==SDIO)
  {
    __HAL_RCC_SDIO_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(SDIO_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);

    __HAL_RCC_DMA2_CLK_ENABLE();

    /* SDIO DMA Init */
    /* SDIO_RX Init */
    hdma_sdio_rx.Instance = DMA2_Stream3;
    hdma_sdio_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sdio_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_rx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_sdio_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_rx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_rx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_rx) != HAL_OK)
    {
      ERROR_PRINTF("SDIO");
    }

    __HAL_LINKDMA(sdHandle,hdmarx,hdma_sdio_rx);

    /* SDIO_TX Init */
    hdma_sdio_tx.Instance = DMA2_Stream6;
    hdma_sdio_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sdio_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_tx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_sdio_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_tx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_tx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_tx) != HAL_OK)
    {
      ERROR_PRINTF("SDIO DMA");
    }

    __HAL_LINKDMA(sdHandle,hdmatx,hdma_sdio_tx);

    
    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef * sdHandle)
  {
    if (sdHandle->Instance == SDIO)
    {
     __HAL_RCC_SDIO_CLK_DISABLE();


      HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

      HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);


      HAL_DMA_DeInit(sdHandle->hdmarx);
      HAL_DMA_DeInit(sdHandle->hdmatx);


      HAL_NVIC_DisableIRQ(SDIO_IRQn);
      HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
      HAL_NVIC_DisableIRQ(DMA2_Stream6_IRQn);

    }
  }
void hal_sd_init(void)
{
  HAL_SD_MspInit(&hsd);
}

void hal_sd_deinit(void)
{
  __HAL_RCC_SDIO_CLK_DISABLE();//클럭만 disable해도 sdio 레지스터 0이된, 이상함.
  __HAL_RCC_SDIO_FORCE_RESET();
  HAL_Delay(1);  // 최소 지연 필요
  __HAL_RCC_SDIO_RELEASE_RESET();

  __HAL_RCC_SDIO_CLK_ENABLE();
  HAL_SD_MspDeInit(&hsd);
}

