

















#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "driver_spi.h"

#include "main.h"

typedef struct spi_api_s
{
  void (*set_cs)(void *handle,void *gpioHandle,uint32_t state);
  void (*send_byte)(void *handle,uint8_t val);
  void (*send_bytes)(void *handle,uint8_t *pData,uint16_t dataLen);
  uint8_t (*read_byte)(void *handle);
  uint8_t (*read_bytes)(void *handle,uint8_t *pData,uint16_t dataLen);
}spi_api_t;



typedef struct spi_cfg_s
{
  void *handle;//STM SPI HANDLE
  void *cs;//STM GPIO HANDLE
  uint32_t pin;
  void *sem;
}spi_cfg_t;

SPI_HandleTypeDef hspi1={.Instance = SPI1};
SPI_HandleTypeDef hspi2={.Instance = SPI2};
   DMA_HandleTypeDef hdma_tx;
   DMA_HandleTypeDef hdma_rx;
static volatile uint32_t SpixTimeout = 0x1000; 


void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  
  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


    
    
    
    
    
    __HAL_RCC_DMA2_CLK_ENABLE();  
    
    
    hdma_tx.Instance                 = DMA2_Stream3;
  
  hdma_tx.Init.Channel             = DMA_CHANNEL_3;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
  hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
  hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
  
  HAL_DMA_Init(&hdma_tx);   
  
  /* Associate the initialized DMA handle to the the SPI handle */
  __HAL_LINKDMA(spiHandle, hdmatx, hdma_tx);
    
  /* Configure the DMA handler for Transmission process */
  hdma_rx.Instance                 = DMA2_Stream0;
  
  hdma_rx.Init.Channel             = DMA_CHANNEL_3;
  hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_rx.Init.Mode                = DMA_NORMAL;
  hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
  hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
  hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4; 

  HAL_DMA_Init(&hdma_rx);
    
  /* Associate the initialized DMA handle to the the SPI handle */
  __HAL_LINKDMA(spiHandle, hdmarx, hdma_rx);
    
  /*##-4- Configure the NVIC for DMA #########################################*/ 
  /* NVIC configuration for DMA transfer complete interrupt (SPI3_TX) */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 1);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
    
  /* NVIC configuration for DMA transfer complete interrupt (SPI3_RX) */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);   
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
  
  /*##-5- Configure the NVIC for SPI #########################################*/
  HAL_NVIC_SetPriority(SPI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(SPI1_IRQn);
    
    
    
    
    
    
    
  }
  else if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* SPI2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOI_CLK_ENABLE();
    /**SPI2 GPIO Configuration
    PI1     ------> SPI2_SCK
    PI2     ------> SPI2_MISO
    PI3     ------> SPI2_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* USER CODE END SPI2_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
  else if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PI1     ------> SPI2_SCK
    PI2     ------> SPI2_MISO
    PI3     ------> SPI2_MOSI
    */
    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}



uint32_t getSPI2ClockFrequency(void) {
    uint32_t systemClock = HAL_RCC_GetSysClockFreq(); // 시스템 클럭 가져오기
    uint32_t apb1Prescaler = (RCC->CFGR & RCC_CFGR_PPRE1) >> 10; // APB1 프리스케일러 추출

    // APB1 프리스케일러 값에 따라 나눗셈 설정
    uint32_t apb1Divider;
    if (apb1Prescaler < 4) {
        apb1Divider = 1; // 프리스케일러 값이 0b000(분주 없음)일 때
    } else {
        apb1Divider = 2 << (apb1Prescaler - 4); // 프리스케일러 값이 0b100(분주 시작)부터
    }

    uint32_t apb1Clock = systemClock / apb1Divider; // APB1 클럭 계산
    return apb1Clock; // SPI2의 메인 클럭 속도 반환
}

// 사용자가 원하는 SPI 클럭에 맞는 baudRatePrescaler 값을 반환하는 함수
uint32_t getBaudRatePrescaler(uint32_t desiredSpiClock) {
    // 프리스케일러 값 테이블 (STM32 HAL에서 사용되는 값)
    const uint32_t prescalers[] = {
        SPI_BAUDRATEPRESCALER_2,
        SPI_BAUDRATEPRESCALER_4,
        SPI_BAUDRATEPRESCALER_8,
        SPI_BAUDRATEPRESCALER_16,
        SPI_BAUDRATEPRESCALER_32,
        SPI_BAUDRATEPRESCALER_64,
        SPI_BAUDRATEPRESCALER_128,
        SPI_BAUDRATEPRESCALER_256
    };
    uint32_t pclk = getSPI2ClockFrequency();

    // 실제 분주 값 테이블
    const uint32_t actualDivisors[] = {2, 4, 8, 16, 32, 64, 128, 256};

    for (int i = 0; i < 8; i++) {
        // PCLK를 현재 분주 값으로 나눈 SPI 클럭 속도
        uint32_t calculatedSpiClock = pclk / actualDivisors[i];
        if (calculatedSpiClock < desiredSpiClock) {
            return prescalers[i+1]; // 적절한 prescaler 반환
        }
    }

    // 원하는 SPI 클럭 속도가 너무 낮을 경우 최대 프리스케일러 반환
    return SPI_BAUDRATEPRESCALER_256;
}



void stm32_spi_init(SPI_HandleTypeDef *hspi)
{
  if(hspi->Instance == SPI1)
  {
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = getBaudRatePrescaler(2000000);;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
          Error_Handler(__FILE__,__LINE__);;
    }
  }
  else if(hspi->Instance == SPI2)
  {
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = getBaudRatePrescaler(2000000);
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
  //      Error_Handler(__FILE__,__LINE__);
  }
  }



}






void stm32_spi_send_byte(void *hspi, uint8_t value)
{
	HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit((SPI_HandleTypeDef *)hspi, (uint8_t*) &value, 1, SpixTimeout);

	if(status != HAL_OK)
	{
		//MSP_SPIx_Error(hspi);
	}
}

#if 1
void stm32_spi_send_bytes(void *hspi,uint8_t *data,uint16_t dataLen)
{
  HAL_StatusTypeDef status;

	status = HAL_SPI_Transmit((SPI_HandleTypeDef*)hspi, (uint8_t*) data, dataLen, SpixTimeout);

	if(status != HAL_OK)
	{
	//	MSP_SPIx_Error(hspi);
	}
}

#else
void stm32_spi_send_bytes(void *hspi,uint8_t *data,uint16_t dataLen)
{
  HAL_StatusTypeDef status;

  if(HAL_SPI_Transmit_DMA((SPI_HandleTypeDef*)hspi, (uint8_t*)data, dataLen) != HAL_OK)
  {
    /* Transfer error in transmission process */
    //    Error_Handler(__FILE__,__LINE__);
  }
}
#endif



uint8_t stm32_spi_read_byte(void *hspi)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t readvalue=0xff;
  
  
  status = HAL_SPI_Receive((SPI_HandleTypeDef *)hspi, (uint8_t*) &readvalue, 1, SpixTimeout);


	if(status != HAL_OK)
	{
	//	MSP_SPIx_Error(hspi);
	}

	return readvalue;

}

#if 1 
uint8_t stm32_spi_read_bytes(void *hspi,uint8_t *pBuff,uint16_t rLen)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t readvalue=0xff;
  
  
  status = HAL_SPI_Receive((SPI_HandleTypeDef *)hspi, (uint8_t*)pBuff, rLen, SpixTimeout);


	if(status != HAL_OK)
	{
	//	MSP_SPIx_Error(hspi);
    return 1;
	}

	return 0;

}
#else
uint8_t stm32_spi_read_bytes(void *hspi,uint8_t *pBuff,uint16_t rLen)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t readvalue=0xff;
  
  
  HAL_SPI_Receive_DMA((SPI_HandleTypeDef *)hspi, pBuff,rLen);

	if(status != HAL_OK)
	{
	//	MSP_SPIx_Error(hspi);
	}

	return readvalue;

}

#endif







spi_api_t g_spi_api2 = {.send_byte  = stm32_spi_send_byte,
                      .send_bytes = stm32_spi_send_bytes,
                      .read_byte  =  stm32_spi_read_byte,
                      .read_bytes = stm32_spi_read_bytes};

spi_cfg_t g_spi1_cfg ={.handle = &hspi1,
                       .cs = OUT_SPI2_NSS_GPIO_Port,
                       .pin = OUT_SPI2_NSS_Pin};

spi_cfg_t g_spi2_cfg ={.handle = &hspi2,
                       .cs = OUT_SPI2_NSS_GPIO_Port,
                       .pin = OUT_SPI2_NSS_Pin};



void driver_spi_init(driver_spi_t *spi,uint32_t num)
{
  switch(num)
  {

      case STM_SPI_2:
      spi->api = (void *)&g_spi_api2;
      spi->apiCfg = &g_spi2_cfg;
      if(spi->sem == NULL)
      {
        spi->sem = osSemaphoreNew(1, 1, NULL); 
      }

      stm32_spi_init(&hspi2);
      break;
    
  }
}


void driver_spi_send_byte(driver_spi_t *spi, uint8_t value)
{
  spi_api_t *api = (spi_api_t *)spi->api;
  spi_cfg_t *cfg = (spi_cfg_t *)spi->apiCfg;

  api->send_byte(cfg->handle,value);
}


void driver_spi_send_bytes(driver_spi_t *spi,uint8_t *data,uint16_t dataLen)
{
  spi_api_t *api = (spi_api_t *)spi->api;
  spi_cfg_t *cfg = (spi_cfg_t *)spi->apiCfg;

  api->send_bytes(cfg->handle,data,dataLen);
}

uint8_t driver_spi_read_byte(driver_spi_t *spi)
{
  uint8_t data;

    spi_api_t *api = (spi_api_t *)spi->api;
  spi_cfg_t *cfg = (spi_cfg_t *)spi->apiCfg;

  data = api->read_byte(cfg->handle);

  return data;
}







static stm32_spi_cfg_t stm32_spi_cfg;
static driver_t spi1={.opened=false};
static spi_api_t g_spi_api={.send_byte  = stm32_spi_send_byte,
                            .send_bytes = stm32_spi_send_bytes,
                            .read_byte  = stm32_spi_read_byte,
                            .read_bytes = stm32_spi_read_bytes};

driver_t *driver_spi_open(int num)
{
  switch(num)
  {
    case STM_SPI_1:
    if(spi1.opened == false)
    {
      spi1.opened = true;

      stm32_spi_init(&hspi1);
      stm32_spi_cfg.handle = &hspi1;
      spi1.cfg = &stm32_spi_cfg;
      spi1.api = &g_spi_api;
      if(spi1.sem == NULL)
      {
        spi1.sem = osSemaphoreNew(1, 1, NULL); 
      }
    }
    return &spi1;
    break;
  }
  
  return 0;

}

void driverex_spi_send_byte(driver_t *spi, uint8_t value)
{
  spi_api_t *api = (spi_api_t*)spi->api;

  if(spi->sem)
  {
   // osSemaphoreAcquire(spi->sem, osWaitForever);
  }
  api->send_byte((( stm32_spi_cfg_t*)spi->cfg)->handle,value);


  if(spi->sem)
  {
    //osSemaphoreRelease(spi->sem);
  }
}

void driverex_spi_send_bytes(driver_t *spi,uint8_t *data,uint16_t dataLen)
{
  spi_api_t *api = (spi_api_t*)spi->api;

  api->send_bytes((( stm32_spi_cfg_t*)spi->cfg)->handle,data,dataLen);


}

uint8_t driverex_spi_read_byte(driver_t *spi)
{
  uint8_t data;
  spi_api_t *api = (spi_api_t*)spi->api;

  data = api->read_byte((( stm32_spi_cfg_t*)spi->cfg)->handle);

  return data;
}



uint8_t driverex_spi_read_bytes(driver_t *spi,uint8_t *pBuff,uint16_t rLen)
{
  uint8_t data;

  spi_api_t *api = (spi_api_t*)spi->api;

  api->read_bytes((( stm32_spi_cfg_t*)spi->cfg)->handle,pBuff,rLen);

    return 0;

}



void driverex_spi_pend_sem(driver_t *spi)
{
  if(spi->sem)
  {
    osSemaphoreAcquire(spi->sem, osWaitForever);
  }
}

void driverex_spi_post_sem(driver_t *spi)
{
  if(spi->sem)
  {
    osSemaphoreRelease(spi->sem);
  }
}



//tx
void DMA1_Stream4_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hspi1.hdmatx);
}


void SPI1_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&hspi1);
}


 void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_RxCpltCallback should be implemented in the user file
   */
}
 void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_TxRxCpltCallback should be implemented in the user file
   */
}
 void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_TxCpltCallback should be implemented in the user file
   */
}

 void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_ErrorCallback should be implemented in the user file
   */
  /* NOTE : The ErrorCode parameter in the hspi handle is updated by the SPI processes
            and user can use HAL_SPI_GetError() API to check the latest error occurred
   */
}

 void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_AbortCpltCallback can be implemented in the user file.
   */
}