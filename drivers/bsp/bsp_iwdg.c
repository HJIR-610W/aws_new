#include "bsp.h"
#include "system_err.h"


IWDG_HandleTypeDef IwdgHandle;
static __IO uint32_t g_uwLsiFreq = 0;
TIM_HandleTypeDef  TimInputCaptureHandle;
__IO uint32_t uwCaptureNumber = 0;
__IO uint32_t uwPeriodValue = 0;
__IO uint32_t uwMeasurementDone = 0;
uint16_t tmpCC4[2] = {0, 0};

void iwdg_callback(TIM_HandleTypeDef *htim)
{
  /* Get the Input Capture value */
  tmpCC4[uwCaptureNumber++] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);

  if (uwCaptureNumber >= 2)
  {
    /* Compute the period length */
    uwPeriodValue = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);
    uwMeasurementDone = 1;
    uwCaptureNumber = 0;
  }
}
/**
  * @brief  Configures TIM5 to measure the LSI oscillator frequency.
  * @param  None
  * @retval LSI Frequency
  */
static uint32_t GetLSIFrequency(void)
{
  uint32_t pclk1 = 0, latency = 0;
  TIM_IC_InitTypeDef timinputconfig = {0};
  RCC_OscInitTypeDef oscinit = {0};
  RCC_ClkInitTypeDef  clkinit =  {0};
  
  /* Enable LSI Oscillator */
  oscinit.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  oscinit.LSIState = RCC_LSI_ON;
  oscinit.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&oscinit)!= HAL_OK)
  {
 //   Error_Handler(); 
  }

  /* Configure the TIM peripheral */
  /* Set TIMx instance */
  TimInputCaptureHandle.Instance = TIM5;

  /* TIMx configuration: Input Capture mode ---------------------
  The LSI clock is connected to TIM5 CH4.
  The Rising edge is used as active edge.
  The TIM5 CCR4 is used to compute the frequency value.
  ------------------------------------------------------------ */
  TimInputCaptureHandle.Init.Prescaler         = 0;
  TimInputCaptureHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimInputCaptureHandle.Init.Period            = 0xFFFF;
  TimInputCaptureHandle.Init.ClockDivision     = 0;
  TimInputCaptureHandle.Init.RepetitionCounter = 0;

  if (HAL_TIM_IC_Init(&TimInputCaptureHandle) != HAL_OK)
  {
    /* Initialization Error */
   // Error_Handler();
  }
  /* Connect internally the  TIM5 CH4 Input Capture to the LSI clock output */
  HAL_TIMEx_RemapConfig(&TimInputCaptureHandle, TIM_TIM5_LSI);

  /* Configure the Input Capture of channel 4 */
  timinputconfig.ICPolarity  = TIM_ICPOLARITY_RISING;
  timinputconfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
  timinputconfig.ICPrescaler = TIM_ICPSC_DIV8;
  timinputconfig.ICFilter    = 0;

  if (HAL_TIM_IC_ConfigChannel(&TimInputCaptureHandle, &timinputconfig, TIM_CHANNEL_4) != HAL_OK)
  {
    /* Initialization Error */
   // Error_Handler();
  }

  /* Reset the flags */
  TimInputCaptureHandle.Instance->SR = 0;

  /* Start the TIM Input Capture measurement in interrupt mode */
  if (HAL_TIM_IC_Start_IT(&TimInputCaptureHandle, TIM_CHANNEL_4) != HAL_OK)
  {
    /* Starting Error */
  //  Error_Handler();
  }

  /* Wait until the TIM5 get 2 LSI edges (refer to TIM5_IRQHandler() in
  stm32f4xx_it.c file) */
  while (uwMeasurementDone == 0)
  {
  }
  uwCaptureNumber = 0;

  /* Deinitialize the TIM5 peripheral registers to their default reset values */
  HAL_TIM_IC_DeInit(&TimInputCaptureHandle);

  /* Compute the LSI frequency, depending on TIM5 input clock frequency (PCLK1)*/
  /* Get PCLK1 frequency */
  pclk1 = HAL_RCC_GetPCLK1Freq();
  HAL_RCC_GetClockConfig(&clkinit, &latency);

  /* Get PCLK1 prescaler */
  if ((clkinit.APB1CLKDivider) == RCC_HCLK_DIV1)
  {
    /* PCLK1 prescaler equal to 1 => TIMCLK = PCLK1 */
    return ((pclk1 * 8) / uwPeriodValue);
  }
  else
  {
    /* PCLK1 prescaler different from 1 => TIMCLK = 2 * PCLK1 */
    return (((2 * pclk1) * 8) / uwPeriodValue) ;
  }
}



void bsp_iwdg_init(uint32_t timeout_ms)
{
  // 1. LSI 주파수 측정
  if (g_uwLsiFreq == 0)
  {
    g_uwLsiFreq = 32000;//GetLSIFrequency(); // 예: 32000Hz
  }

  // 가능한 프리스케일러 값 배열
  uint32_t prescaler_values[] = {4, 8, 16, 32, 64, 128, 256};
  uint32_t prescaler_bits[] = {
      IWDG_PRESCALER_4,
      IWDG_PRESCALER_8,
      IWDG_PRESCALER_16,
      IWDG_PRESCALER_32,
      IWDG_PRESCALER_64,
      IWDG_PRESCALER_128,
      IWDG_PRESCALER_256};

  uint32_t selected_prescaler = 0;
  uint32_t selected_reload = 0;

  // 2. 적절한 Prescaler 및 Reload 값 계산
  /*
   timeout_ms  =  (reload +1) *prescaler/LSI *1000 
   */
  for (int i = 0; i < 7; i++)
  {
    uint32_t prescaler = prescaler_values[i];
    uint32_t reload = ((timeout_ms * g_uwLsiFreq) / (1000 * prescaler)) - 1;

    if (reload <= 0x0FFF)
    {
      selected_prescaler = prescaler_bits[i];
      selected_reload = reload;
      break;
    }
  }

  // 3. IWDG 초기화
  IwdgHandle.Instance = IWDG;
  IwdgHandle.Init.Prescaler = selected_prescaler;
  IwdgHandle.Init.Reload = selected_reload;

  if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
  {

  }
}

void bsp_iwdg_reload(void)
{
  /*
여러 개의 리로드 값 또는 프리스케일러 값을 애플리케이션에서 사용하는 경우, 리로드 값을 변경하기 전에 RVU 비트가 리셋될 때까지, 
프리스케일러 값을 변경하기 전에 PVU 비트가 리셋될 때까지 반드시 대기해야 합니다.
그러나, 프리스케일러 및/또는 리로드 값을 업데이트한 후에는 RVU 또는 PVU 비트가 리셋될 때까지 기다릴 필요 없이 코드 실행을 계속할 수 있습니다
(저전력 모드로 진입하는 경우에도, 쓰기 작업은 고려되어 완료됩니다).

*/
  HAL_IWDG_Refresh(&IwdgHandle) ;
}

