



#include "driver_freqInput.h"

#include <math.h>

#include "os_user_def.h"
#include "pcb_define.h"



#ifdef PCB_0_5


#define TIM_FREQ 10000

typedef struct freq_cfg_s
{
  uint8_t channel;

} freq_cfg_t;

driver_t g_freqMeasure[FREQ_MAX];
freq_cfg_t g_freq_cfg[FREQ_MAX];

TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;

static float g_freq_TIM10 = 0.0f;
static float g_freq_TIM11 = 0.0f;



static uint32_t last_rising_TIM10 = 0;
static uint32_t current_rising_TIM10 = 0;

static uint32_t last_rising_TIM11 = 0;
static uint32_t current_rising_TIM11 = 0;

uint32_t last_capture_tick_TIM10 = 0;
uint32_t last_capture_tick_TIM11 = 0;

uint32_t calculate_timer_prescaler(TIM_TypeDef *tim_instance, uint32_t desired_freq_hz);


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
  GPIO_InitStruct.Pin = TIM10_CH1_Pin;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM10;
  HAL_GPIO_Init(TIM10_CH1_GPIO_Port, &GPIO_InitStruct);

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
  GPIO_InitStruct.Pin = TIM11_CH1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM11;
  HAL_GPIO_Init(TIM11_CH1_GPIO_Port, &GPIO_InitStruct);

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



driver_t *driver_freq_open(uint32_t num)
{
  if (g_freqMeasure[num].opened == true)
  {
    return &g_freqMeasure[num];
  }

  g_freqMeasure[num].opened = true;
  switch (num)
  {
    case FREQ_MEAURE_B:
      freqMeasureB_init();
      g_freq_cfg[FREQ_MEAURE_B].channel = 0;
      g_freqMeasure[num].cfg = &g_freq_cfg[FREQ_MEAURE_B];


      break;
    case FREQ_MEAURE_C:
      freqMeasureC_init();
      g_freq_cfg[FREQ_MEAURE_C].channel = 1;
      g_freqMeasure[num].cfg = &g_freq_cfg[FREQ_MEAURE_C];

      break;
  }

  return &g_freqMeasure[num];
}


#define FREQ_MEASURE_TIMEOUT 3000
float driver_freq_read(driver_t *drv,uint8_t *err)
{
  freq_cfg_t *cfg = drv->cfg;
  *err = 0;

  if (cfg->channel == 0)
  {
    if ((HAL_GetTick() -  last_capture_tick_TIM10)>FREQ_MEASURE_TIMEOUT)
    {
      return 0.0f;
    }
    else
    {
      return g_freq_TIM10;
    }
  }
  else if (cfg->channel == 1)
  {
    if ((HAL_GetTick() - last_capture_tick_TIM11) > FREQ_MEASURE_TIMEOUT)
    {
      return 0.0f;
    }
    else
    {
      return g_freq_TIM11;
    }
  }

  return 0;
}

float driver_freq_read_duty(driver_t *drv,uint8_t *err)
{
  *err = 0;
  return NAN;  // Duty cycle measurement disabled
}

#endif


#ifdef PCB_0_6
#include "driver_freqInput.h"

#include <math.h>

#include "os_user_def.h"
#include "pcb_define.h"


#define TIM5_CH1_Pin GPIO_PIN_0
#define TIM5_CH1_GPIO_Port GPIOA
#define TIM2_CH1_ETR_Pin GPIO_PIN_5
#define TIM2_CH1_ETR_GPIO_Port GPIOA


#define TIMER_FREQUENCY 10000
typedef struct freq_cfg_s
{
  uint8_t channel;

} freq_cfg_t;

driver_t g_freqMeasure[FREQ_MAX];
freq_cfg_t g_freq_cfg[FREQ_MAX];

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
  __HAL_RCC_TIM5_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin = TIM5_CH1_Pin;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
  HAL_GPIO_Init(TIM5_CH1_GPIO_Port, &GPIO_InitStruct);

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = calculate_timer_prescaler(TIM5, TIMER_FREQUENCY); 
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 0xFFFFFFFF; // TIM2 is 32-bit
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

void freqMeasureC_init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM2_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = TIM2_CH1_ETR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(TIM2_CH1_ETR_GPIO_Port, &GPIO_InitStruct);

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = calculate_timer_prescaler(TIM2, TIMER_FREQUENCY); 
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF; 
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_IC_Init(&htim2);

  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;  
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

  HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void) 
{
  HAL_TIM_IRQHandler(&htim2); 
}

void TIM5_IRQHandler(void) 
{
  HAL_TIM_IRQHandler(&htim5); 
}

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



driver_t *driver_freq_open(uint32_t num)
{
  if (g_freqMeasure[num].opened == true)
  {
    return &g_freqMeasure[num];
  }

  g_freqMeasure[num].opened = true;
  switch (num)
  {
    case FREQ_MEAURE_B:
      freqMeasureB_init();
      g_freq_cfg[FREQ_MEAURE_B].channel = 0;
      g_freqMeasure[num].cfg = &g_freq_cfg[FREQ_MEAURE_B];
      OS_CREATE_BINARY_SEM(g_freqMeasure[num].sem);

      break;
    case FREQ_MEAURE_C:
      freqMeasureC_init();
      g_freq_cfg[FREQ_MEAURE_C].channel = 1;
      g_freqMeasure[num].cfg = &g_freq_cfg[FREQ_MEAURE_C];
      OS_CREATE_BINARY_SEM(g_freqMeasure[num].sem);
      break;
  }

  return &g_freqMeasure[num];
}

#define FREQ_MEASURE_TIMEOUT 3000
float driver_freq_read(driver_t *drv, uint8_t *err)
{
  freq_cfg_t *cfg = drv->cfg;
  *err = 0;

  if (cfg->channel == 0)
  {
    if ((HAL_GetTick() - last_capture_tick_TIM5) > FREQ_MEASURE_TIMEOUT)
    {
      return 0.0f;
    }
    else
    {
      return g_freq_TIM5;
    }
  }
  else if (cfg->channel == 1)
  {
    if ((HAL_GetTick() - last_capture_tick_TIM2) > FREQ_MEASURE_TIMEOUT)
    {
      return 0.0f;
    }
    else
    {
      return g_freq_TIM2;
    }
  }

  return 0;
}

float driver_freq_read_duty(driver_t *drv,uint8_t *err)
{
  *err = 0;
  return NAN;  // Duty cycle measurement disabled
}


#endif