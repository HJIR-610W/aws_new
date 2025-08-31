
#include "cmsis_os2.h"
#include "pcb_define.h"
#include "driver_stm32_adc.h"
#include "system_err.h"
#include "os_user_def.h"

typedef struct
{
  ADC_HandleTypeDef hadc;
  void *adcIrqSem;
  bool opened;
  void *sem;
}adc_instance_t;

adc_instance_t adc_instance = {.hadc.Instance = ADC1};
osSemaphoreId_t adcSemaphore;
volatile uint32_t adcValue = 0;

ADC_HandleTypeDef *get_adc_handle(void)
{
  return &adc_instance.hadc;
}

void  MX_ADC_Init(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    hadc->Instance = ADC1;
    hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc->Init.Resolution = ADC_RESOLUTION_12B;
    hadc->Init.ScanConvMode = DISABLE;
    hadc->Init.ContinuousConvMode = DISABLE;
    hadc->Init.DiscontinuousConvMode = DISABLE;
    hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc->Init.NbrOfConversion = 1;
    hadc->Init.DMAContinuousRequests = DISABLE;
    hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  }
  if (HAL_ADC_Init(hadc) != HAL_OK)
  {
    ERROR_PRINTF("adc");
  }

  HAL_NVIC_SetPriority(ADC_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(ADC_IRQn);

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3|GPIO_PIN_4);


  }
}


// ADC 변환 완료 인터럽트 콜백
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    adcValue = HAL_ADC_GetValue(hadc);
    osSemaphoreRelease(adc_instance.adcIrqSem);  // 세마포어 해제
  }
}



#define ADC_TIMEOUT_MS  100

/*
채널 0: ADC_CHANNEL_3
채널 1: ADC_CHANNEL_4
*/
int32_t stm32_adc_read_single(int channel,uint16_t average_count,uint8_t *err)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  uint32_t sum = 0;
  int32_t avg;

  osSemaphoreAcquire(adc_instance.sem, osWaitForever);

  if (channel == 0)
    sConfig.Channel = ADC_CHANNEL_3;
  else if (channel == 1)
    sConfig.Channel = ADC_CHANNEL_4;
  else
    return 0;  // 지원되지 않는 채널

  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

  if (HAL_ADC_ConfigChannel(&adc_instance.hadc, &sConfig) != HAL_OK)
  {
    osSemaphoreRelease(adc_instance.sem);  // 세마포어 해제
    return 0;
  }

  
  for (uint32_t i = 0; i < average_count; i++)
  {
    HAL_ADC_Start_IT(&adc_instance.hadc);  // ADC 변환 시작 (인터럽트 사용)

    // 변환 완료까지 대기 (세마포어)
    if (osSemaphoreAcquire(adc_instance.adcIrqSem, ADC_TIMEOUT_MS) != osOK)
    {
      osSemaphoreRelease(adc_instance.sem);  // 세마포어 해제
      return 0;  // 타임아웃 발생 시 0 반환
    }

    sum += adcValue;
  }

  avg = sum/average_count;

  osSemaphoreRelease(adc_instance.sem);  // 세마포어 해제
  
  return avg;  // 평균값 반환
}


void stm32_adc_init(void)
{
  if (adc_instance.opened)
  {
    return;
  }

  OS_CREATE_BINARY_SEM(adc_instance.sem);

  if (adc_instance.adcIrqSem == NULL)
  {
    adc_instance.adcIrqSem = osSemaphoreNew(1, 0, NULL);
  }

  MX_ADC_Init(&adc_instance.hadc);

  adc_instance.opened = true;
}
