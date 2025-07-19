#include "driver_stm32_frequency.h"
#include "cmsis_os2.h"
#include "pcb_define.h"
#include "system_err.h"
#include "os_user_def.h"

#ifndef PCB_0_6
// PCB_0_6이 정의되지 않은 경우: TIM10, TIM11 사용
#define TIM_FREQ 10000

typedef struct
{
  TIM_HandleTypeDef htim;
  float frequency;
  uint32_t last_rising;
  uint32_t current_rising;
  uint32_t last_capture_tick;
  bool measuring;
  void *sem;
} freq_instance_t;

static freq_instance_t freq_instances[STM32_FREQ_MAX] = {0};

TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;

float g_freq_TIM10 = 0.0f;
float g_freq_TIM11 = 0.0f;

// Frequency measurement variables - TIM10
static uint32_t last_rising_TIM10 = 0;
static uint32_t current_rising_TIM10 = 0;

// Frequency measurement variables - TIM11
static uint32_t last_rising_TIM11 = 0;
static uint32_t current_rising_TIM11 = 0;

uint32_t last_capture_tick_TIM10 = 0;
uint32_t last_capture_tick_TIM11 = 0;

uint32_t calculate_timer_prescaler(TIM_TypeDef *tim_instance, uint32_t desired_freq_hz);

// Capture callback function - frequency measurement only
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM10 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    current_rising_TIM10 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    last_capture_tick_TIM10 = HAL_GetTick();

    if (last_rising_TIM10 != 0)
    {
      uint32_t delta = (current_rising_TIM10 >= last_rising_TIM10) ? 
                      (current_rising_TIM10 - last_rising_TIM10) :
                      (0xFFFF - last_rising_TIM10 + current_rising_TIM10 + 1);

      if (delta > 0)
        g_freq_TIM10 = (float)TIM_FREQ / delta;
    }

    last_rising_TIM10 = current_rising_TIM10;
  }
  else if (htim->Instance == TIM11 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    current_rising_TIM11 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    last_capture_tick_TIM11 = HAL_GetTick();

    if (last_rising_TIM11 != 0)
    {
      uint32_t delta = (current_rising_TIM11 >= last_rising_TIM11) ? 
                      (current_rising_TIM11 - last_rising_TIM11) :
                      (0xFFFF - last_rising_TIM11 + current_rising_TIM11 + 1);

      if (delta > 0)
        g_freq_TIM11 = (float)TIM_FREQ / delta;
    }

    last_rising_TIM11 = current_rising_TIM11;
  }
}

void freqMeasureB_init(void)
{
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_TIM10_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin = IN_TIM10_CH1_PIN;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM10;
  HAL_GPIO_Init(IN_TIM10_CH1_GPIO_Port, &GPIO_InitStruct);

  htim10.Instance = TIM10;
  htim10.Init.Prescaler = calculate_timer_prescaler(TIM10, TIM_FREQ);  // 10kHz timer frequency (100us period)
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 0xFFFF;
  HAL_TIM_IC_Init(&htim10);

  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;  // Detect rising edge only
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim10, &sConfigIC, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim10, TIM_CHANNEL_1);

  HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}

// Initialization function - TIM11
void freqMeasureC_init(void)
{
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_TIM11_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = IN_TIM11_CH1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM11;
  HAL_GPIO_Init(IN_TIM11_CH1_GPIO_Port, &GPIO_InitStruct);

  htim11.Instance = TIM11;
  htim11.Init.Prescaler = calculate_timer_prescaler(TIM11, TIM_FREQ);  // 10kHz timer frequency (100us period)
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 0xFFFF;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_IC_Init(&htim11);

  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;  // Detect rising edge only
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim11, &sConfigIC, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim11, TIM_CHANNEL_1);

  HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
}

void TIM1_UP_TIM10_IRQHandler(void) 
{
  HAL_TIM_IRQHandler(&htim10); 
}

void TIM1_TRG_COM_TIM11_IRQHandler(void) 
{
  HAL_TIM_IRQHandler(&htim11); 
}

#else
// PCB_0_6이 정의된 경우: TIM2, TIM5 사용
#define TIMER_FREQUENCY 100000

typedef struct
{
  TIM_HandleTypeDef htim;
  float frequency;
  uint32_t last_rising;
  uint32_t current_rising;
  uint32_t last_capture_tick;
  bool measuring;
  void *sem;
} freq_instance_t;

static freq_instance_t freq_instances[STM32_FREQ_MAX] = {0};

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim5;

float g_freq_TIM2 = 0.0f;
float g_freq_TIM5 = 0.0f;

// Frequency measurement variables - TIM2
static uint32_t last_rising_TIM2 = 0;
static uint32_t current_rising_TIM2 = 0;

// Frequency measurement variables - TIM5
static uint32_t last_rising_TIM5 = 0;
static uint32_t current_rising_TIM5 = 0;

uint32_t last_capture_tick_TIM2 = 0;
uint32_t last_capture_tick_TIM5 = 0;

uint32_t calculate_timer_prescaler(TIM_TypeDef *tim_instance, uint32_t desired_freq_hz);

// Capture callback function - frequency measurement only
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    current_rising_TIM2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    last_capture_tick_TIM2 = HAL_GetTick();

    if (last_rising_TIM2 != 0)
    {
      uint32_t delta = (current_rising_TIM2 >= last_rising_TIM2) ? 
                      (current_rising_TIM2 - last_rising_TIM2) :
                      (0xFFFFFFFF - last_rising_TIM2 + current_rising_TIM2 + 1);

      if (delta > 0)
        g_freq_TIM2 = (float)TIMER_FREQUENCY / delta;
    }

    last_rising_TIM2 = current_rising_TIM2;
  }
  else if (htim->Instance == TIM5 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    current_rising_TIM5 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    last_capture_tick_TIM5 = HAL_GetTick();

    if (last_rising_TIM5 != 0)
    {
      uint32_t delta = (current_rising_TIM5 >= last_rising_TIM5) ? 
                      (current_rising_TIM5 - last_rising_TIM5) :
                      (0xFFFFFFFF - last_rising_TIM5 + current_rising_TIM5 + 1);

      if (delta > 0)
        g_freq_TIM5 = (float)TIMER_FREQUENCY / delta;
    }

    last_rising_TIM5 = current_rising_TIM5;
  }
}

void freqMeasureB_init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM2_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin = GPIO_PIN_5;  // IN_TIM2_CH1_PIN
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // IN_TIM2_CH1_GPIO_Port

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = calculate_timer_prescaler(TIM2, TIMER_FREQUENCY);  // 100kHz timer frequency (10us period)
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF;  // TIM2 is 32-bit
  HAL_TIM_IC_Init(&htim2);

  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;  // Detect rising edge only
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

  HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

// Initialization function - TIM5
void freqMeasureC_init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM5_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_0;  // IN_TIM5_CH1_PIN
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // IN_TIM5_CH1_GPIO_Port

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = calculate_timer_prescaler(TIM5, TIMER_FREQUENCY);  // 100kHz timer frequency (10us period)
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 0xFFFFFFFF;  // TIM5 is 32-bit
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_IC_Init(&htim5);

  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;  // Detect rising edge only
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1);

  HAL_NVIC_SetPriority(TIM5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM5_IRQn);
}

void TIM2_IRQHandler(void) 
{
  HAL_TIM_IRQHandler(&htim2); 
}

void TIM5_IRQHandler(void) 
{
  HAL_TIM_IRQHandler(&htim5); 
}

#endif

// Calculate prescaler value for desired timer resolution frequency
uint32_t calculate_timer_prescaler(TIM_TypeDef *tim_instance, uint32_t desired_freq_hz)
{
  uint32_t timer_clock_hz = 0;
  
  // Get timer clock source frequency
  if (tim_instance == TIM2 || tim_instance == TIM5)
  {
    // TIM2 and TIM5 are on APB1 bus
    timer_clock_hz = HAL_RCC_GetPCLK1Freq();
    
    // Check if APB1 prescaler is > 1, then timer clock is doubled
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0)
    {
      timer_clock_hz *= 2;
    }
  }
  else if (tim_instance == TIM10 || tim_instance == TIM11)
  {
    // TIM10 and TIM11 are on APB2 bus
    timer_clock_hz = HAL_RCC_GetPCLK2Freq();
    
    // Check if APB2 prescaler is > 1, then timer clock is doubled
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != 0)
    {
      timer_clock_hz *= 2;
    }
  }
  else if (tim_instance == TIM13)
  {
    // TIM13 is on APB1 bus
    timer_clock_hz = HAL_RCC_GetPCLK1Freq();
    
    // Check if APB1 prescaler is > 1, then timer clock is doubled
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0)
    {
      timer_clock_hz *= 2;
    }
  }
  
  // Calculate prescaler: Timer_Clock / Desired_Freq - 1
  uint32_t prescaler = (timer_clock_hz / desired_freq_hz) - 1;
  
  // Ensure prescaler is within valid range (0-65535 for 16-bit timers)
  if (prescaler > 0xFFFF)
  {
    prescaler = 0xFFFF;
  }
  
  return prescaler;
}

void stm32_frequency_init(void)
{
  for (int i = 0; i < STM32_FREQ_MAX; i++)
  {
    if (!freq_instances[i].sem)
    {
      OS_CREATE_BINARY_SEM(freq_instances[i].sem);
    }
    
    freq_instances[i].measuring = false;
    freq_instances[i].frequency = 0.0f;
  }

  // Initialize channels based on available timers
#ifndef PCB_0_6
  // Use TIM10, TIM11
  freqMeasureB_init();  // Channel 0
  freqMeasureC_init();  // Channel 1
#else
  // Use TIM2, TIM5
  freqMeasureB_init();  // Channel 0
  freqMeasureC_init();  // Channel 1
#endif
}

float stm32_frequency_read(int channel, uint8_t *err)
{
  if (channel < 0 || channel >= STM32_FREQ_MAX)
  {
    if (err) *err = 1;
    return 0.0f;
  }

#ifndef PCB_0_6
  if (channel == 0)
  {
    if (err) *err = 0;
    return g_freq_TIM10;
  }
  else if (channel == 1)
  {
    if (err) *err = 0;
    return g_freq_TIM11;
  }
#else
  if (channel == 0)
  {
    if (err) *err = 0;
    return g_freq_TIM2;
  }
  else if (channel == 1)
  {
    if (err) *err = 0;
    return g_freq_TIM5;
  }
#endif

  if (err) *err = 1;
  return 0.0f;
}

void stm32_frequency_start_measurement(int channel)
{
  if (channel < 0 || channel >= STM32_FREQ_MAX)
  {
    return;
  }

  freq_instances[channel].measuring = true;
  freq_instances[channel].frequency = 0.0f;
  
#ifndef PCB_0_6
  if (channel == 0)
  {
    HAL_TIM_IC_Start_IT(&htim10, TIM_CHANNEL_1);
  }
  else if (channel == 1)
  {
    HAL_TIM_IC_Start_IT(&htim11, TIM_CHANNEL_1);
  }
#else
  if (channel == 0)
  {
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  }
  else if (channel == 1)
  {
    HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1);
  }
#endif
}

void stm32_frequency_stop_measurement(int channel)
{
  if (channel < 0 || channel >= STM32_FREQ_MAX)
  {
    return;
  }

  freq_instances[channel].measuring = false;

#ifndef PCB_0_6
  if (channel == 0)
  {
    HAL_TIM_IC_Stop_IT(&htim10, TIM_CHANNEL_1);
  }
  else if (channel == 1)
  {
    HAL_TIM_IC_Stop_IT(&htim11, TIM_CHANNEL_1);
  }
#else
  if (channel == 0)
  {
    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
  }
  else if (channel == 1)
  {
    HAL_TIM_IC_Stop_IT(&htim5, TIM_CHANNEL_1);
  }
#endif
}

bool stm32_frequency_is_measuring(int channel)
{
  if (channel < 0 || channel >= STM32_FREQ_MAX)
  {
    return false;
  }

  return freq_instances[channel].measuring;
}