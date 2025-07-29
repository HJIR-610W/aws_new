
#include "bsp.h"
#include "bsp_led.h"

typedef struct  led_inst_s
{
  GPIO_TypeDef *port;
  uint16_t pin;
} led_inst_t;

led_inst_t led_inst[BSP_LED_MAX] = {
    [BSP_LED_RUN].port = DO_SYS_RUN_GPIO_Port, [BSP_LED_RUN].pin = DO_SYS_RUN_Pin};

TIM_HandleTypeDef htim12;

void bsp_led_set_blink_freq(int led_number, int freq, int high_duty);


    // TIM12 클럭 주파수 계산 함수
    uint32_t Get_TIM12_ClockFrequency(void)
{
    uint32_t timer_clock;
    uint32_t apb1_prescaler = (RCC->CFGR & RCC_CFGR_PPRE1) >> 10; // APB1 프리스케일러 값 추출

    if (apb1_prescaler < 4) 
    {
        // APB1 프리스케일러가 1인 경우, 타이머 클럭 = APB1 클럭
        timer_clock = HAL_RCC_GetPCLK1Freq();
    }
    else 
    {
        // APB1 프리스케일러가 2 이상인 경우, 타이머 클럭 = APB1 클럭 * 2
        timer_clock = HAL_RCC_GetPCLK1Freq() * 2;
    }

    return timer_clock;
}

void Set_PWM_Frequency(uint32_t frequency, uint8_t duty_cycle)
{
    uint32_t timer_clock = Get_TIM12_ClockFrequency();  // TIM12의 클럭 주파수
    uint32_t prescaler;
    uint32_t arr;

    // `Prescaler`를 먼저 크게 설정하여 `ARR`이 16비트 내에 들어오도록 조정
    for (prescaler = 1; prescaler <= 65536; prescaler++)
    {
        arr = (timer_clock / (prescaler * frequency)) - 1;

        // `ARR` 값이 16비트를 초과하지 않으면 루프를 종료
        if (arr <= 65535)
        {
            break;
        }
    }

    // TIM12의 분주기(Prescaler)와 ARR(주기) 설정
    __HAL_TIM_SET_PRESCALER(&htim12, prescaler-1);  // Prescaler 설정
    __HAL_TIM_SET_AUTORELOAD(&htim12, arr);       // ARR 설정

    // 듀티 사이클 설정: CCR2 = (ARR + 1) * (듀티 사이클 %) / 100
    uint32_t ccr_value = ((arr + 1) * duty_cycle) / 100;
    __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, ccr_value); // CCR2에 듀티 설정
}



void TIM12_PWM_Init(void)
{
    __HAL_RCC_TIM12_CLK_ENABLE();  // TIM12 클럭 활성화

    // TIM12 PWM 설정 (기본 설정)
    TIM_OC_InitTypeDef sConfigOC;
    htim12.Instance = TIM12;
    htim12.Init.Prescaler = 0;
    htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim12.Init.Period = 1000 - 1;  // 기본 주기 설정 (ARR)
    htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim12);

    // PWM 모드 설정 (채널 2)
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 500;  // Duty cycle 50%
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_2);

    // PWM 시작 (PH9 핀)

}

void led_run_init(void)
{
    // GPIO 포트 H 클럭 활성화
    __HAL_RCC_GPIOH_CLK_ENABLE();
  
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PH9 핀 설정: TIM12 채널 2 대체 기능 (AF) 모드
    GPIO_InitStruct.Pin = DO_SYS_RUN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;      // 대체 기능, 푸시 풀 출력
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // 풀업/풀다운 비활성화
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // 속도 설정
    GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;  // TIM12의 대체 기능 9번 설정
    HAL_GPIO_Init(DO_SYS_RUN_GPIO_Port, &GPIO_InitStruct);

    TIM12_PWM_Init();
}



void bsp_led_init(void)
{
  for (int led_number = 0; led_number < sizeof(led_inst) / sizeof(led_inst_t); led_number++)
  {
    switch (led_number)
    {
      case BSP_LED_RUN:
        led_run_init();
        bsp_led_set_blink_freq(BSP_LED_RUN,1,10);
        break;
    }
  }

}



void bsp_led_on(int led_number)
{
  switch (led_number)
  {
  case BSP_LED_RUN:
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
    break;
  default:
     HAL_GPIO_WritePin(led_inst[led_number].port, led_inst[led_number].pin,GPIO_PIN_RESET);
    break;
  }


}

void bsp_led_off(int led_number)
{
  switch (led_number)
  {
    case BSP_LED_RUN:
      HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
      break;
    default:
      HAL_GPIO_WritePin(led_inst[led_number].port, led_inst[led_number].pin, GPIO_PIN_SET);
      break;
  }

}

void bsp_led_toggle(int led_number)
{
  switch (led_number)
  {
    case BSP_LED_RUN:
      break;
    default:
      HAL_GPIO_TogglePin(led_inst[led_number].port, led_inst[led_number].pin);
      break;
  }

}


void bsp_led_set_blink_freq(int led_number,int freq,int high_duty)
{
  switch (led_number)
  {
  case BSP_LED_RUN:
    Set_PWM_Frequency(freq,high_duty);
    break;

  default:
    break;
  }
}