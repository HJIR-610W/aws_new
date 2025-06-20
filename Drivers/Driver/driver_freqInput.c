
#include "driver_freqInput.h"

#include "os_user_def.h"
#include "stm32f4xx_hal.h"

#define IN_TIM10_CH1_Pin GPIO_PIN_6
#define IN_TIM10_CH1_GPIO_Port GPIOF

#define IN_TIM11_CH1_Pin GPIO_PIN_7
#define IN_TIM11_CH1_GPIO_Port GPIOF

#define IN_TIM13_CH1_Pin GPIO_PIN_8
#define IN_TIM13_CH1_GPIO_Port GPIOF

typedef struct ds1306_cfg_s
{
  uint8_t channel;

} freq_cfg_t;

driver_t g_freqMeasure[FREQ_MAX];
freq_cfg_t g_freq_cfg[FREQ_MAX];

float g_freq[FREQ_MAX];
float g_duty[FREQ_MAX];
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;
TIM_HandleTypeDef htim13;

float g_freq_TIM10 = 0.0f;
float g_duty_TIM10 = 0.0f;
float g_freq_TIM11 = 0.0f;
float g_duty_TIM11 = 0.0f;

// 인터럽트용 변수 - TIM10
static uint32_t rising_edge = 0;
static uint32_t falling_edge = 0;
static uint32_t last_rising = 0;
static uint8_t last_edge = 0;  // 0: none, 1: rising, 2: falling

// 인터럽트용 변수 - TIM11
static uint32_t rising_edge_11 = 0;
static uint32_t falling_edge_11 = 0;
static uint32_t last_rising_11 = 0;
static uint8_t last_edge_11 = 0;

uint32_t last_capture_tick_TIM10 = 0;
uint32_t last_capture_tick_TIM11 = 0;

// 캡처 콜백 함수
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM10 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    uint32_t currCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    uint32_t delta = 0;

    last_capture_tick_TIM10 = HAL_GetTick();

    if (last_edge != 1)
    {
      rising_edge = currCapture;

      if (last_rising != 0)
      {
        delta = (rising_edge >= last_rising) ? (rising_edge - last_rising)
                                             : (0xFFFF - last_rising + rising_edge + 1);

        if (delta > 0)
          g_freq_TIM10 = 10000.0f / delta;

        if (falling_edge != 0 && falling_edge != rising_edge)
        {
          uint32_t high_time = (falling_edge >= last_rising)
                                   ? (falling_edge - last_rising)
                                   : (0xFFFF - last_rising + falling_edge + 1);

          g_duty_TIM10 = (float)high_time * 100.0f / delta;
        }
      }

      last_rising = rising_edge;
      last_edge = 1;
    }
    else
    {
      falling_edge = currCapture;
      last_edge = 2;
    }
  }
  else if (htim->Instance == TIM11 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    uint32_t currCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    uint32_t delta = 0;
    last_capture_tick_TIM11 = HAL_GetTick();

    if (last_edge_11 != 1)
    {
      rising_edge_11 = currCapture;

      if (last_rising_11 != 0)
      {
        delta = (rising_edge_11 >= last_rising_11) ? (rising_edge_11 - last_rising_11)
                                                   : (0xFFFF - last_rising_11 + rising_edge_11 + 1);

        if (delta > 0)
          g_freq_TIM11 = 10000.0f / delta;

        if (falling_edge_11 != 0 && falling_edge_11 != rising_edge_11)
        {
          uint32_t high_time = (falling_edge_11 >= last_rising_11)
                                   ? (falling_edge_11 - last_rising_11)
                                   : (0xFFFF - last_rising_11 + falling_edge_11 + 1);

          g_duty_TIM11 = (float)high_time * 100.0f / delta;
        }
      }

      last_rising_11 = rising_edge_11;
      last_edge_11 = 1;
    }
    else
    {
      falling_edge_11 = currCapture;
      last_edge_11 = 2;
    }
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
  GPIO_InitStruct.Pin = IN_TIM10_CH1_Pin;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM10;
  HAL_GPIO_Init(IN_TIM10_CH1_GPIO_Port, &GPIO_InitStruct);

  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 8399;  // 100us 타이머 주기
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 0xFFFF;
  HAL_TIM_IC_Init(&htim10);

  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_ICPOLARITY_BOTHEDGE;  // 상승/하강 모두 측정
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim10, &sConfigIC, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim10, TIM_CHANNEL_1);

  HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}
// 초기화 함수 - TIM11
void freqMeasureC_init(void)
{
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_TIM11_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = IN_TIM11_CH1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM11;
  HAL_GPIO_Init(IN_TIM11_CH1_GPIO_Port, &GPIO_InitStruct);

  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 8399;  // 100us 타이머 주기
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 0xFFFF;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_IC_Init(&htim11);

  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_ICPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim11, &sConfigIC, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim11, TIM_CHANNEL_1);

  HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
}

void TIM1_UP_TIM10_IRQHandler(void) { HAL_TIM_IRQHandler(&htim10); }

void TIM1_TRG_COM_TIM11_IRQHandler(void) { HAL_TIM_IRQHandler(&htim11); }

void TIM8_UP_TIM13_IRQHandler(void) { HAL_TIM_IRQHandler(&htim13); }

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
      break;
  }

  return &g_freqMeasure[num];
}

#define FREQ_TIMEOUT_MS 1000

float driver_freq_read(driver_t *drv,uint8_t *err)
{
  freq_cfg_t *cfg = drv->cfg;
  *err = 0;

  if (cfg->channel == 0)
  {
    if(1)// (osSemaphoreAcquire(drv->sem, FREQ_TIMEOUT_MS) == osOK)
    {
      return g_freq_TIM10;
    }
    else
    {
      g_freq_TIM10 = 0.0f;
      return 0.0f;
    }
  }
  else if (cfg->channel == 1)
  {
    if(1)// (osSemaphoreAcquire(drv->sem, FREQ_TIMEOUT_MS) == osOK)
    {
      return g_freq_TIM10;
    }
    else
    {
      g_freq_TIM11 = 0.0f;
      return 0.0f;
    }
  }

  return 0;
}

float driver_freq_read_duty(driver_t *drv,uint8_t *err)
{
  freq_cfg_t *cfg = drv->cfg;
  *err = 0;

  if (cfg->channel == 0)
  {
    if (HAL_GetTick() - last_capture_tick_TIM10 > 5000)
      return 0.0f;

    return g_duty_TIM10;
  }
  else if (cfg->channel == 1)
  {
    if (HAL_GetTick() - last_capture_tick_TIM11 > 5000)
      return 0.0f;
    return g_duty_TIM11;
  }

  return 0;
}