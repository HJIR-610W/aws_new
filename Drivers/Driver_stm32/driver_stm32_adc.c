
#include "cmsis_os2.h"
#include "pcb_define.h"
#include "driver_stm32_adc.h"
#include "system_err.h"

typedef struct
{
  ADC_HandleTypeDef  *handle;
  void *adcIrqSem;
}stm32_adc_config_t;




ADC_HandleTypeDef  hadc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

 

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    ERROR_PRINTF("adc");
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */

  /* USER CODE BEGIN ADC1_Init 2 */
  HAL_NVIC_SetPriority(ADC_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(ADC_IRQn);
  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA3     ------> ADC1_IN3
    PA4     ------> ADC1_IN4
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA3     ------> ADC1_IN3
    PA4     ------> ADC1_IN4
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3|GPIO_PIN_4);

  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}


driver_t stm32_adc_driver;
stm32_adc_config_t stm32_adc_config={.handle = &hadc1};

osSemaphoreId_t adcSemaphore;
volatile uint32_t adcValue = 0;


void stm32_adc_close(driver_t *handle);
int32_t stm32_adc_read_single(driver_t *handle,int channel,uint16_t avg,uint8_t *err);
int32_t stm32_adc_read_diff(driver_t *handle,int channel,uint16_t avg,uint8_t *err);
void stm32_adc_set(driver_t *handle, adc_set_option_t option, void *value);

adc_api_t adc_api = {.close = stm32_adc_close,
                     .read_single = stm32_adc_read_single,
                     .read_diff = stm32_adc_read_diff,
                     .set = stm32_adc_set};


driver_t *driver_stm32_adc_open(uint32_t num,void *opt)
{
  if(stm32_adc_driver.opened)
  {
    return &stm32_adc_driver;
  }


  stm32_adc_driver.opened = true;
  stm32_adc_driver.name = "STM32_ADC";
  stm32_adc_driver.api = &adc_api;
  stm32_adc_driver.cfg = &stm32_adc_config;

  if(stm32_adc_driver.sem == NULL)
  {
    stm32_adc_driver.sem = osSemaphoreNew(1, 1, NULL); 
  }


  if(stm32_adc_config.adcIrqSem == NULL)
  {
    stm32_adc_config.adcIrqSem = osSemaphoreNew(1, 0, NULL); 
  }
  MX_ADC1_Init();
  return &stm32_adc_driver;

}



void stm32_adc_close(driver_t *handle)
{

}


// ADC 변환 완료 인터럽트 콜백
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    adcValue = HAL_ADC_GetValue(hadc);
    osSemaphoreRelease(stm32_adc_config.adcIrqSem);  // 세마포어 해제
  }
}

void iar_adcCompleted(void *driver)
{
  driver_t *drv = driver;
    stm32_adc_config_t *cfg = drv->cfg;
    
  if (cfg->handle->Instance == ADC1)
  {
    adcValue = HAL_ADC_GetValue(cfg->handle);
    osSemaphoreRelease(stm32_adc_config.adcIrqSem);  // 세마포어 해제
  }
}


#define ADC_TIMEOUT_MS  100

/*
채널 0: ADC_CHANNEL_3
채널 1: ADC_CHANNEL_4
*/
int32_t stm32_adc_read_single(driver_t *handle,int channel,uint16_t avgCnt,uint8_t *err)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  stm32_adc_config_t *cfg = handle->cfg;
  uint32_t sum = 0;

  int32_t avg;

  osSemaphoreAcquire(handle->sem, osWaitForever);

  if (channel == 0)
    sConfig.Channel = ADC_CHANNEL_3;
  else if (channel == 1)
    sConfig.Channel = ADC_CHANNEL_4;
  else
    return 0;  // 지원되지 않는 채널

  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

  if (HAL_ADC_ConfigChannel(cfg->handle, &sConfig) != HAL_OK)
  {
     osSemaphoreRelease(handle->sem);  // 세마포어 해제
    return 0;
  }

  
  for (uint32_t i = 0; i < avgCnt; i++)
  {
    HAL_ADC_Start_IT(cfg->handle);  // ADC 변환 시작 (인터럽트 사용)

    // 변환 완료까지 대기 (세마포어)
    if (osSemaphoreAcquire(cfg->adcIrqSem, ADC_TIMEOUT_MS) != osOK)
    {
      osSemaphoreRelease(handle->sem);  // 세마포어 해제
      return 0;  // 타임아웃 발생 시 0 반환
    }

    sum += adcValue;
  }

  avg = sum/avgCnt;

   osSemaphoreRelease(handle->sem);  // 세마포어 해제
  return avg;  // 평균값 반환
}


int32_t stm32_adc_read_diff(driver_t *handle,int channel,uint16_t avg,uint8_t *err)
{

return 0;
}

void stm32_adc_set(driver_t *handle, adc_set_option_t option, void *value)
{

}