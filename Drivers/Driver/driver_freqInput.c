
#include "driver_freqInput.h"

#include "stm32f4xx_hal.h"


#define IN_TIM10_CH1_Pin GPIO_PIN_6
#define IN_TIM10_CH1_GPIO_Port GPIOF

#define IN_TIM11_CH1_Pin GPIO_PIN_7
#define IN_TIM11_CH1_GPIO_Port GPIOF

#define IN_TIM13_CH1_Pin GPIO_PIN_8
#define IN_TIM13_CH1_GPIO_Port GPIOF

driver_t g_freqMeasure[3];


float g_freq[3];
float g_duty[3];
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;
TIM_HandleTypeDef htim13;

volatile uint32_t capture_val_10 = 0;
volatile uint32_t capture_val_11 = 0;
volatile uint32_t capture_val_13 = 0;

typedef struct {
    float frequency;
    float period;
} FrequencyPeriodResult;

typedef struct {
    float frequency;
    float period;
    float duty_cycle;
} FrequencyDutyCycleResult;

volatile uint32_t capture_val_prev1 = 0;
volatile uint32_t capture_val_prev2 = 0;
volatile uint32_t capture_val_prev3 = 0;

volatile uint32_t capture_rising = 0;     // 상승 에지에서 캡처한 값
volatile uint32_t capture_falling = 0;    // 하강 에지에서 캡처한 값
volatile uint8_t is_rising_edge = 1;      // 현재 에지 상태 (1이면 상승 에지, 0이면 하강 에지)

            float measured_frequency ;
            float measured_period ;
            float measured_duty_cycle;

// 주기와 듀티 사이클 계산 함수
FrequencyDutyCycleResult Calculate_Frequency_DutyCycle(TIM_HandleTypeDef *htim)
{
    FrequencyDutyCycleResult result = {0.0f, 0.0f, 0.0f};
    uint32_t timer_clock = HAL_RCC_GetPCLK2Freq();  // 타이머 클럭 주파수

    uint32_t period_ticks = capture_falling - capture_rising;
    uint32_t high_ticks = capture_falling - capture_rising;

    // 주기 계산
    result.period = (float)period_ticks / timer_clock;
    
    // 주파수 계산
    if (result.period > 0.0f) {
        result.frequency = 1.0f / result.period;
    }

    // 듀티 사이클 계산
    result.duty_cycle = ((float)high_ticks / (float)period_ticks) * 100.0f;

    return result;
}


// 타이머 캡처 인터럽트 콜백
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM10) 
    {
        uint32_t new_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

    }
    else if (htim->Instance == TIM11)
    {
        uint32_t new_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);


    }
    else if (htim->Instance == TIM13)
    {
        uint32_t capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        
        if (is_rising_edge) {
            capture_rising = capture_val;   // 상승 에지에서 캡처
            is_rising_edge = 0;             // 다음 캡처는 하강 에지에서 수행
        } else {
            capture_falling = capture_val;  // 하강 에지에서 캡처
            is_rising_edge = 1;             // 다음 캡처는 상승 에지에서 수행

            // 듀티 사이클과 주파수 계산
            FrequencyDutyCycleResult result = Calculate_Frequency_DutyCycle(htim);
            
             measured_frequency = result.frequency;
             measured_period = result.period;
             measured_duty_cycle = result.duty_cycle;
            
            // 계산된 주기, 주파수, 듀티 사이클 값을 사용할 수 있음
        }
    }
}


void freqMeasureA_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  // GPIOF Pin 8 설정 (TIM13_CH1)
  GPIO_InitStruct.Pin = IN_TIM13_CH1_Pin;
  GPIO_InitStruct.Alternate = GPIO_AF9_TIM13;
  HAL_GPIO_Init(IN_TIM13_CH1_GPIO_Port, &GPIO_InitStruct);

  __HAL_RCC_TIM13_CLK_ENABLE();
  htim13.Instance = TIM13;
  htim13.Init.Prescaler = 83;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim13.Init.Period = 0xFFFF;
  HAL_TIM_IC_Init(&htim13);

  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_ICPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim13, &sConfigIC, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim13, TIM_CHANNEL_1);


    HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
}

void freqMeasureB_init(void)
{
    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

        // GPIOF Pin 6 설정 (TIM10_CH1)
    GPIO_InitStruct.Pin = IN_TIM10_CH1_Pin;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM10;
    HAL_GPIO_Init(IN_TIM10_CH1_GPIO_Port, &GPIO_InitStruct);

 __HAL_RCC_TIM10_CLK_ENABLE();
    htim10.Instance = TIM10;
    htim10.Init.Prescaler = 83; // 84MHz / (Prescaler + 1) = 1MHz (1us 타이머 주기)
    htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim10.Init.Period = 0xFFFF;
    HAL_TIM_IC_Init(&htim10);

    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim10, &sConfigIC, TIM_CHANNEL_1);

    HAL_TIM_IC_Start_IT(&htim10, TIM_CHANNEL_1);

    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}
void freqMeasureC_init(void)
{
      GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    // GPIOF Pin 7 설정 (TIM11_CH1)
    GPIO_InitStruct.Pin = IN_TIM11_CH1_Pin;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM11;
    HAL_GPIO_Init(IN_TIM11_CH1_GPIO_Port, &GPIO_InitStruct);


     __HAL_RCC_TIM11_CLK_ENABLE();
    htim11.Instance = TIM11;
    htim11.Init.Prescaler = 83;
    htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim11.Init.Period = 0xFFFF;
    HAL_TIM_IC_Init(&htim11);

    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim11, &sConfigIC, TIM_CHANNEL_1);

    HAL_TIM_IC_Start_IT(&htim11, TIM_CHANNEL_1);


       HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

}

void TIM1_UP_TIM10_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim10);
}

void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim11);
}

void TIM8_UP_TIM13_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim13);
}

driver_t *driver_freq_open(uint32_t num)
{
  if(g_freqMeasure[num].opened == true)
  {
    return &g_freqMeasure[num];
  }


  g_freqMeasure[num].opened = true;
  switch(num)
  {
    case FREQ_MEAURE_A:
      freqMeasureA_init();
    break;
    case FREQ_MEAURE_B:
      freqMeasureB_init();
    break;
    case FREQ_MEAURE_C:
      freqMeasureC_init();
    break;
  }

  //  g_freqMeasure[num].num  = num;
    return &g_freqMeasure[num];
}



void driver_freq_read(driver_t *drv,float *freq)
{
 // *freq = g_freq[drv->num];
}