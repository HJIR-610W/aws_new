
#include "bsp.h"

#include <math.h>  // isnan() 사용을 위해 필요
#include "bsp_crc.h"
#include "bsp_di.h"
#include "bsp_do.h"
#include "bsp_i2c.h"
#include "bsp_delay.h"
#include "bsp_adc.h"
#include "bsp_uart.h"
#include "bsp_rs485.h"
#include "bsp_eth.h"
#include "config_app.h"
#include "drv_di.h"
#include "drv_do.h"
#include "drv_led.h"
#include "fsmc.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "system_err.h"
#include "test_sram.h"
#include "tlsf.h"
#include "user_heap.h"


uint32_t g_pcb_version = AWS_PCB_VERSION;

TIM_HandleTypeDef        htim4;

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock, uwAPB1Prescaler = 0U;

  uint32_t              uwPrescalerValue = 0U;
  uint32_t              pFLatency;
  HAL_StatusTypeDef     status;

  /* Enable TIM4 clock */
  __HAL_RCC_TIM4_CLK_ENABLE();

  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  /* Get APB1 prescaler */
  uwAPB1Prescaler = clkconfig.APB1CLKDivider;
  /* Compute TIM4 clock */
  if (uwAPB1Prescaler == RCC_HCLK_DIV1)
  {
    uwTimclock = HAL_RCC_GetPCLK1Freq();
  }
  else
  {
    uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
  }

  /* Compute the prescaler value to have TIM4 counter clock equal to 1MHz */
  uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

  /* Initialize TIM4 */
  htim4.Instance = TIM4;

  /* Initialize TIMx peripheral as follow:

  + Period = [(TIM4CLK/1000) - 1]. to have a (1/1000) s time base.
  + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
  + ClockDivision = 0
  + Counter direction = Up
  */
  htim4.Init.Period = (1000000U / 1000U) - 1U;
  htim4.Init.Prescaler = uwPrescalerValue;
  htim4.Init.ClockDivision = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  status = HAL_TIM_Base_Init(&htim4);
  if (status == HAL_OK)
  {
    /* Start the TIM time Base generation in interrupt mode */
    status = HAL_TIM_Base_Start_IT(&htim4);
    if (status == HAL_OK)
    {
    /* Enable the TIM4 global Interrupt */
        HAL_NVIC_EnableIRQ(TIM4_IRQn);
      /* Configure the SysTick IRQ priority */
      if (TickPriority < (1UL << __NVIC_PRIO_BITS))
      {
        /* Configure the TIM IRQ priority */
        HAL_NVIC_SetPriority(TIM4_IRQn, TickPriority, 0U);
        uwTickPrio = TickPriority;
      }
      else
      {
        status = HAL_ERROR;
      }
    }
  }

 /* Return function status */
  return status;
}

/**
  * @brief  Suspend Tick increment.
  * @note   Disable the tick increment by disabling TIM4 update interrupt.
  * @param  None
  * @retval None
  */
void HAL_SuspendTick(void)
{
  /* Disable TIM4 update Interrupt */
  __HAL_TIM_DISABLE_IT(&htim4, TIM_IT_UPDATE);
}

/**
  * @brief  Resume Tick increment.
  * @note   Enable the tick increment by Enabling TIM4 update interrupt.
  * @param  None
  * @retval None
  */
void HAL_ResumeTick(void)
{
  /* Enable TIM4 Update interrupt */
  __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);
}



void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

}









/*
공급전압 최대 입력을 15V로 하자
0~2.5V => 0~15V
12V(전압)
|
49.9K
|-------1K---ADC
10K
|
GND
*/


#define BATTERY_AVERAGE_SAMPLES 50

float bsp_read_battery(void)
{
  const float slope = 6;  // (float)(15.0f-0.0f)/(float)(2.5-0);
  const float offset = 0.0;
  uint8_t err;
  float voltage;
  float battery;

  voltage = bsp_adc_single_read_voltage(BSP_ADC_SYS_BATTERY, BATTERY_AVERAGE_SAMPLES, &err);

  // voltage가 NaN인 경우, NaN 반환
  if (isnan(voltage))
  {
    return NAN;
  }

  // voltage가 NaN이 아닌 경우 배터리 계산
  battery = voltage * slope + offset;

  return battery;
}



/*
온도측정정
3.3V(VREF)
|
10K(R1)
|----------ADC
10K(NTC)
|
GND

25도라면 3.3V/2 = 1.65v가 ADC되어야함
3.3V *(NTC/(R1+NTC)) = ADC전압값
NTC = (ADC*R1)/(3.3V-ADC)
*/

#define VREF 3.3f           // ADC 기준 전압
#define R1 10000.0f   // 10kΩ 풀업 저항

typedef struct {
  float temperature;
  float resistance;
} NTC_Lookup;

/*
LNSK16G103 NTC써미스터
10kΩ (25도 기준)
온도에 따라 저항이 변함
온도가 높아질수록 저항이 감소
*/
const NTC_Lookup ntc_table[] = {
  { -40.0, 200800 }, { -35.0, 152900 }, { -30.0, 117200 }, { -25.0, 90510 },
  { -20.0, 70400 }, { -15.0, 55140 }, { -10.0, 43510 }, { -5.0, 34570 },
  { 0.0, 27660 }, { 5.0, 22280 }, { 10.0, 18070 }, { 15.0, 14740 },
  { 20.0, 12110 }, { 25.0, 10000 }, { 30.0, 8307 }, { 35.0, 6938 },
  { 40.0, 5824 }, { 45.0, 4913 }, { 50.0, 4164 }, { 55.0, 3543 },
  { 60.0, 3028 }, { 65.0, 2597 }, { 70.0, 2235 }, { 75.0, 1930 },
  { 80.0, 1671 }, { 85.0, 1452 }, { 90.0, 1264 }, { 95.0, 1104 },
  { 100.0, 966 }, { 105.0, 848 }, { 110.0, 746 }, { 115.0, 657 },
  { 120.0, 581 }
};
#define TABLE_SIZE (sizeof(ntc_table) / sizeof(ntc_table[0]))

float ntc_resistance_to_temperature(float resistance)
{
  if (resistance >= ntc_table[0].resistance)
    return ntc_table[0].temperature;  // 최소 온도 이하
  if (resistance <= ntc_table[TABLE_SIZE - 1].resistance)
    return ntc_table[TABLE_SIZE - 1].temperature;  // 최대 온도 이상

  // 테이블에서 적절한 범위를 찾음
  for (int i = 0; i < TABLE_SIZE - 1; i++)
  {
    if (resistance <= ntc_table[i].resistance && resistance > ntc_table[i + 1].resistance)
    {
      // 선형 보간법 적용
      float temp1 = ntc_table[i].temperature;
      float temp2 = ntc_table[i + 1].temperature;
      float res1 = ntc_table[i].resistance;
      float res2 = ntc_table[i + 1].resistance;

      // y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)
      return (temp1 + (resistance - res1) * (temp2 - temp1) / (res2 - res1));
    }
  }

  return 0.0f; // 이론적으로 도달하지 않음
}



#define TEMP_AVERAGE_SAMPLES 50

float bsp_read_temperature(void)
{
  uint8_t err;
  float voltage;
  float resistance;

  voltage = bsp_adc_single_read_voltage(BSP_ADC_SYS_TEMP, TEMP_AVERAGE_SAMPLES, &err);

  // voltage가 NaN인 경우, NaN 반환
  if (isnan(voltage))
  {
    return NAN;
  }

  // voltage가 NaN이 아닌 경우에만 저항 계산
  resistance = (voltage * R1) / (VREF - voltage);

  return ntc_resistance_to_temperature(resistance);
}



/*
GPIOx의 클럭이 enable 안되어 있으면 enable 해줌
*/
void board_clk_gpio(GPIO_TypeDef *GPIOx)
{
  if(GPIOx == GPIOA && (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN) == 0))
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  }
  else if(GPIOx == GPIOB && (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN) == 0))
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }
  else if(GPIOx == GPIOC && (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN) == 0))
  {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }
  else if(GPIOx == GPIOD && (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN) == 0))
  {
    __HAL_RCC_GPIOD_CLK_ENABLE();
  }
  else if(GPIOx == GPIOE && (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOEEN) == 0))
  {
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
  else if(GPIOx == GPIOF && (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOFEN) == 0))
  {
    __HAL_RCC_GPIOF_CLK_ENABLE();
  }
  else if(GPIOx == GPIOG && (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOGEN) == 0))
  {
    __HAL_RCC_GPIOG_CLK_ENABLE();
  }
  else if(GPIOx == GPIOH && (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOHEN) == 0))
  {
    __HAL_RCC_GPIOH_CLK_ENABLE();
  }
  else if(GPIOx == GPIOI && (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOIEN) == 0))
  {
    __HAL_RCC_GPIOI_CLK_ENABLE();
  }
}


void board_set_gpio(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
  board_clk_gpio(GPIOx);
  HAL_GPIO_WritePin(GPIOx,GPIO_Pin,PinState);
}

void board_config_gpio(GPIO_TypeDef *GPIOx,uint32_t pin,uint32_t mode,uint32_t pull,uint32_t speed,uint32_t alternate)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  board_clk_gpio(GPIOx);

  GPIO_InitStruct.Pin   = pin;
  GPIO_InitStruct.Mode  = mode;
  GPIO_InitStruct.Pull  = pull;
  GPIO_InitStruct.Speed = speed;
  GPIO_InitStruct.Alternate = alternate;
  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

// APB2 타이머 클럭 주파수 계산
 uint32_t get_apb2_timer_clock(void)
{
  uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();

  // APB2 프리스케일러가 1이 아닌 경우 타이머 클럭은 PCLK2 × 2
  // APB2 프리스케일러가 1인 경우 타이머 클럭은 PCLK2와 동일
  uint32_t ppre2 = (RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos;

  if (ppre2 == 0)
  {
    // 분주 없음 (APB2 프리스케일러 = 1)
    return pclk2;
  }
  else
  {
    // 분주 있음 (APB2 프리스케일러 > 1)
    return pclk2 * 2;
  }
}


extern void manual_bss_init(void);




void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
}

/*
보드가사용하는 핀을 초기화 한다.
drv에서도 초기화가 되지만 drv가 config 설정에 의해 사용하지 않는 경우도 있기에 
반드시 초기화 해줘야 하는 핀이 있다면 여기서 초기화 해준다.
*/
void bsp_gpio_init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  // QUAD UART 칩을 리셋해준다. H->L
  board_set_gpio(DO_EX_UART_RST_A_GPIO_Port, DO_EX_UART_RST_A_Pin, GPIO_PIN_SET);
  board_config_gpio(DO_EX_UART_RST_A_GPIO_Port, DO_EX_UART_RST_A_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);

  board_set_gpio(DO_EX_UART_RST_B_GPIO_Port, DO_EX_UART_RST_B_Pin, GPIO_PIN_SET);
  board_config_gpio(DO_EX_UART_RST_B_GPIO_Port, DO_EX_UART_RST_B_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);
  HAL_Delay(10);

  board_set_gpio(DO_EX_UART_RST_A_GPIO_Port, DO_EX_UART_RST_A_Pin, GPIO_PIN_RESET);
  board_set_gpio(DO_EX_UART_RST_B_GPIO_Port, DO_EX_UART_RST_B_Pin, GPIO_PIN_RESET);

  //[RS485] RS485 A 방향 입력 설정
  board_set_gpio(DO_DIR_RS485_A_GPIO_Port, DO_DIR_RS485_A_Pin, GPIO_PIN_RESET);
  board_config_gpio(DO_DIR_RS485_A_GPIO_Port, DO_DIR_RS485_A_Pin, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

  //[RS485] RS485 B 방향 입력 설정
  board_set_gpio(DO_DIR_RS485_B_GPIO_Port, DO_DIR_RS485_B_Pin, GPIO_PIN_RESET);
  board_config_gpio(DO_DIR_RS485_B_GPIO_Port, DO_DIR_RS485_B_Pin, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

  //[RS485] RS485 C 방향 입력 설정
  board_set_gpio(DO_RS485_DIR_C_GPIO_Port, DO_RS485_DIR_C_Pin, GPIO_PIN_RESET);
  board_config_gpio(DO_RS485_DIR_C_GPIO_Port, DO_RS485_DIR_C_Pin, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

  //[RS485] RS485 D 방향 입력 설정
  board_set_gpio(DO_RS485_DIR_D_GPIO_Port, DO_RS485_DIR_D_Pin, GPIO_PIN_RESET);
  board_config_gpio(DO_RS485_DIR_D_GPIO_Port, DO_RS485_DIR_D_Pin, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

  //[SPI CS] FRAM 
  board_set_gpio(DO_SPI1_NSS_GPIO_Port, DO_SPI1_NSS_Pin, GPIO_PIN_SET);
  board_config_gpio(DO_SPI1_NSS_GPIO_Port, DO_SPI1_NSS_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);
 
  //[SPI CS] ADC
  board_set_gpio(DO_SPI2_NSS_GPIO_Port, DO_SPI2_NSS_Pin, GPIO_PIN_SET);
  board_config_gpio(DO_SPI2_NSS_GPIO_Port, DO_SPI2_NSS_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);

  //[SPI CS] FLASH
  board_set_gpio(DO_CS_S_FLASH_GPIO_Port, DO_CS_S_FLASH_Pin, GPIO_PIN_SET);
  board_config_gpio(DO_CS_S_FLASH_GPIO_Port, DO_CS_S_FLASH_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);
  
  // UART_5_EXT_D 는 HART와 RS232 선택사용 포트이다. RS232를 기본설정한다.
  board_set_gpio(DO_SEL_IF_UART_GPIO_Port, DO_SEL_IF_UART_Pin, GPIO_PIN_RESET);
  board_config_gpio(DO_SEL_IF_UART_GPIO_Port, DO_SEL_IF_UART_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);

  //[전원]CDMA 12V 전원은 차단한다.
  board_set_gpio(DO_CON_PWR_CDMA_GPIO_Port, DO_CON_PWR_CDMA_Pin, GPIO_PIN_RESET);
  board_config_gpio(DO_CON_PWR_CDMA_GPIO_Port, DO_CON_PWR_CDMA_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);

  //[전원]HART 24V는 차단한다.
  board_set_gpio(DO_CON_PWR_S24_GPIO_Port, DO_CON_PWR_S24_Pin, GPIO_PIN_RESET);
  board_config_gpio(DO_CON_PWR_S24_GPIO_Port, DO_CON_PWR_S24_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);
 
  //[전원]디지털 강우감지 전원은 항상 출력
  board_set_gpio(DO_CON_PWR_RAIN_DIGITAL_GPIO_Port, DO_CON_PWR_RAIN_DIGITAL_Pin,GPIO_PIN_SET);
  board_config_gpio(DO_CON_PWR_RAIN_DIGITAL_GPIO_Port, DO_CON_PWR_RAIN_DIGITAL_Pin,GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);
  //[전원] 아날로그 강우감지 전원 항상 출력
  board_set_gpio(DO_CON_PWR_RAIN_GPIO_Port, DO_CON_PWR_RAIN_Pin, GPIO_PIN_SET);
  board_config_gpio(DO_CON_PWR_RAIN_GPIO_Port, DO_CON_PWR_RAIN_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

  // HART IC를 RESET 상태로 만든다.
  board_set_gpio(DO_RESET_H_GPIO_Port, DO_RESET_H_Pin, GPIO_PIN_RESET);
  board_config_gpio(DO_RESET_H_GPIO_Port, DO_RESET_H_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);

  //[전원]LCD OFF 하드웨어 기본 ON
  board_set_gpio(DO_POWER_LCD_GPIO_Port, DO_POWER_LCD_Pin, GPIO_PIN_SET); 
  board_config_gpio(DO_POWER_LCD_GPIO_Port, DO_POWER_LCD_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);
  
  //LCD 리셋은 활성화
  board_set_gpio(DO_LCD_RESET_GPIO_Port, DO_LCD_RESET_Pin, GPIO_PIN_RESET); 
  board_config_gpio(DO_LCD_RESET_GPIO_Port, DO_LCD_RESET_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);

  //[전원]USB 5v 전원
  board_set_gpio(USB_OTG_FS_SOF_GPIO_Port, USB_OTG_FS_SOF_Pin, GPIO_PIN_SET); 
  board_config_gpio(USB_OTG_FS_SOF_GPIO_Port, USB_OTG_FS_SOF_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, 0);
 


    
}


void bsp_init(void)
{
  bsp_gpio_init();

  MX_FSMC_Init();  // TODO: SRAM초기화,SystemInit_ExtMemCtl 이함수에 적용해야함

  manual_bss_init(); // sram 초기화가 먼저 수행되어야한다.

  user_tlsf_init(POOL_SIZE);//srma 초기화가 먼저 수행되어야한다.에러 발생할수 있음

  bsp_interrupt_init();  // 최우선 실행,연산코드 

  bsp_delay_init();//MCU가 제공하는 정밀 delay

}
