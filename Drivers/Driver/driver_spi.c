

#include "driver_spi.h"

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "system_err.h"

#define SPI_TIME_OUT 0x1000

typedef struct spi_api_s
{
  void (*set_cs)(void *handle, void *gpioHandle, uint32_t state);
  void (*send_byte)(void *handle, uint8_t val);
  void (*send_bytes)(void *handle, uint8_t *pData, uint16_t dataLen);
  uint8_t (*read_byte)(void *handle);
  uint8_t (*read_bytes)(void *handle, uint8_t *pData, uint16_t dataLen);
} spi_api_t;

typedef struct spi_cfg_s
{
  void *handle;  // STM SPI HANDLE
  void *cs;      // STM GPIO HANDLE
  uint32_t pin;
  void *sem;
} spi_cfg_t;


SPI_HandleTypeDef hspi1 = {.Instance = SPI1};
SPI_HandleTypeDef hspi2 = {.Instance = SPI2};


void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if(spiHandle->Instance==SPI1)
  {
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(SPI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
  
  }
  else if(spiHandle->Instance==SPI2)
  {
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);


  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{
  if(spiHandle->Instance==SPI1)
  {
    __HAL_RCC_SPI1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
  }
  else if(spiHandle->Instance==SPI2)
  {
    __HAL_RCC_SPI2_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

  }
}

uint32_t getSPI1ClockFrequency(void) 
{
  uint32_t systemClock = HAL_RCC_GetSysClockFreq(); // 시스템 클럭 가져오기
  uint32_t apb2Prescaler = (RCC->CFGR & RCC_CFGR_PPRE2) >> 13; // APB2 프리스케일러 추출

  // APB2 프리스케일러 값에 따라 나눗셈 설정
  uint32_t apb2Divider;
  if (apb2Prescaler < 4) 
  {
    apb2Divider = 1; // 프리스케일러 값이 0b000(분주 없음)일 때
  } 
  else 
  {
    apb2Divider = 2 << (apb2Prescaler - 4); // 프리스케일러 값이 0b100(분주 시작)부터
  }

  uint32_t apb2Clock = systemClock / apb2Divider; // APB2 클럭 계산
  return apb2Clock; // SPI1의 메인 클럭 속도 반환
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
uint32_t getBaudRatePrescaler(uint32_t desiredSpiClock,uint32_t pclk)
{
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


  // 실제 분주 값 테이블
  const uint32_t actualDivisors[] = {2, 4, 8, 16, 32, 64, 128, 256};

  for (int i = 0; i < 8; i++) {
      // PCLK를 현재 분주 값으로 나눈 SPI 클럭 속도
      uint32_t calculatedSpiClock = pclk / actualDivisors[i];
      if (calculatedSpiClock <= desiredSpiClock) {
          return prescalers[i]; // 적절한 prescaler 반환
      }
  }

  // 원하는 SPI 클럭 속도가 너무 낮을 경우 최대 프리스케일러 반환
  return SPI_BAUDRATEPRESCALER_256;
}

uint32_t get_spi_prescaler(SPI_HandleTypeDef *hspi,uint32_t freq)
{
  uint32_t pclk;
  uint32_t prescale;

  if(hspi->Instance ==SPI1)
  {
    pclk =getSPI1ClockFrequency();
    prescale = getBaudRatePrescaler(freq,pclk);
    
  }
  else if(hspi->Instance ==SPI2)
  {
    pclk = getSPI2ClockFrequency();
    prescale = getBaudRatePrescaler(freq,pclk);
  }

  return prescale;
}



//SPI최대 클럭은 동작클럭의 절반 
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
    hspi1.Init.BaudRatePrescaler = get_spi_prescaler(hspi,10500000);;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
      ERROR_PRINTF("spi");
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
    hspi2.Init.BaudRatePrescaler = get_spi_prescaler(hspi,10500000);;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
      ERROR_PRINTF("spi");
    }
  }

}






void stm32_spi_send_byte(void *hspi, uint8_t value)
{
	HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit((SPI_HandleTypeDef *)hspi, (uint8_t*) &value, 1, SPI_TIME_OUT);

	if(status != HAL_OK)
	{
          ERROR_PRINTF("stm32_spi_send_byte %d", status);
  }
}


void stm32_spi_send_bytes(void *hspi,uint8_t *data,uint16_t dataLen)
{
  HAL_StatusTypeDef status;

	status = HAL_SPI_Transmit((SPI_HandleTypeDef*)hspi, (uint8_t*) data, dataLen, SPI_TIME_OUT);

	if(status != HAL_OK)
	{
          ERROR_PRINTF("stm32_spi_send_bytes %d", status);
  }
}

uint8_t stm32_spi_read_byte(void *hspi)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t readvalue=0x00;
  
  
  status = HAL_SPI_Receive((SPI_HandleTypeDef *)hspi, (uint8_t*) &readvalue, 1, SPI_TIME_OUT);


  if(status != HAL_OK)
  {
    ERROR_PRINTF("stm32_spi_read_byte %d", status);
  }

        return readvalue;

}


uint8_t stm32_spi_read_bytes(void *hspi,uint8_t *pBuff,uint16_t rLen)
{
	HAL_StatusTypeDef status = HAL_OK;
 
  status = HAL_SPI_Receive((SPI_HandleTypeDef *)hspi, (uint8_t*)pBuff, rLen, SPI_TIME_OUT);


	if(status != HAL_OK)
	{
          ERROR_PRINTF("stm32_spi_read_bytes %d", status);
          return 1;
  }

	return 0;

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







static stm32_spi_cfg_t stm32_spi1_cfg;
static stm32_spi_cfg_t stm32_spi2_cfg;
static driver_t spi1={.opened=false};
static driver_t spi2={.opened=false};
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
      spi1.instance_id = num;
      spi1.driver_type = eDRIVER_SPI;
      spi1.name = "STM32_SPI1";
      stm32_spi_init(&hspi1);
      stm32_spi1_cfg.handle = &hspi1;
      spi1.cfg = &stm32_spi1_cfg;
      spi1.api = &g_spi_api;
      if(spi1.sem == NULL)
      {
        spi1.sem = osSemaphoreNew(1, 1, NULL); 
      }
    }
    return &spi1;
    case STM_SPI_2:
    if(spi2.opened == false)
    {
      spi2.opened = true;
      spi2.instance_id = num;
      spi2.driver_type = eDRIVER_SPI;
      spi2.name = "STM32_SPI2";
      stm32_spi_init(&hspi2);
      stm32_spi2_cfg.handle = &hspi2;
      spi2.cfg = &stm32_spi2_cfg;
      spi2.api = &g_spi_api;
      if(spi2.sem == NULL)
      {
        spi2.sem = osSemaphoreNew(1, 1, NULL); 
      }
    }
        return &spi2;
    break;
  }
  
  return 0;

}

void driverex_spi_send_byte(driver_t *spi, uint8_t value)
{
  spi_api_t *api = (spi_api_t*)spi->api;

  api->send_byte((( stm32_spi_cfg_t*)spi->cfg)->handle,value);

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