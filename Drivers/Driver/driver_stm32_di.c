
#include <string.h>
#include "driver_stm32_di.h"
#include "driver_digitalOut.h"
#include "driver_di_def.h"
#include "main.h"
#include "utile.h"
#include "mcu_utile.h"
#include "mcu_interrupt.h"

typedef struct  stm32_di_cfg_s
{
  GPIO_TypeDef *port;
  uint16_t pin;
}stm32_di_cfg_t;

const stm32_di_cfg_t ADC_DRDY_cfg   ={.port=IN_SPI2_DRDY_GPIO_Port,   .pin = IN_SPI2_DRDY_Pin};
const stm32_di_cfg_t RTC_IRQ_cfg    ={.port=INT_RTC_GPIO_Port,        .pin = INT_RTC_Pin};
const stm32_di_cfg_t RAIN_REED_cfg  ={.port=IN_RAIN_REED_GPIO_Port,   .pin = IN_RAIN_REED_Pin};
const stm32_di_cfg_t RAIN_HALL_cfg  ={.port=IN_RAIN_HALL_GPIO_Port,   .pin = IN_RAIN_HALL_Pin};
const stm32_di_cfg_t RAIN_HALL_ERR_cfg  ={.port=IN_RAIN_ERR_GPIO_Port,.pin = IN_RAIN_ERR_Pin};

const stm32_di_cfg_t QUAD_UARTA_1_cfg  ={.port=IN_EX_UART_INT_1_GPIO_Port,.pin = IN_EX_UART_INT_1_Pin};
const stm32_di_cfg_t QUAD_UARTB_2_cfg  ={.port=IN_EX_UART_INT_2_GPIO_Port,.pin = IN_EX_UART_INT_2_Pin};
const stm32_di_cfg_t QUAD_UARTC_3_cfg  ={.port=IN_EX_UART_INT_3_GPIO_Port,.pin = IN_EX_UART_INT_3_Pin};
const stm32_di_cfg_t QUAD_UARTD_4_cfg  ={.port=IN_EX_UART_INT_4_GPIO_Port,.pin = IN_EX_UART_INT_4_Pin};
const stm32_di_cfg_t QUAD_UARTA_5_cfg  ={.port=IN_EX_UART_INT_5_GPIO_Port,.pin = IN_EX_UART_INT_5_Pin};
const stm32_di_cfg_t QUAD_UARTB_6_cfg  ={.port=IN_EX_UART_INT_6_GPIO_Port,.pin = IN_EX_UART_INT_6_Pin};
const stm32_di_cfg_t QUAD_UARTC_7_cfg  ={.port=IN_EX_UART_INT_7_GPIO_Port,.pin = IN_EX_UART_INT_7_Pin};
const stm32_di_cfg_t QUAD_UARTD_8_cfg  ={.port=IN_EX_UART_INT_8_GPIO_Port,.pin = IN_EX_UART_INT_8_Pin};


driver_t g_stm32_di_list[STM32_DI_MAX];

void stm32_di_init(const stm32_di_cfg_t *cfg)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  board_clk_gpio(cfg->port);
  GPIO_InitStruct.Pin = cfg->pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(cfg->port ,&GPIO_InitStruct);

}

driver_t *stm32_di_open(int num)
{

  if(g_stm32_di_list[num].opened)
  {
    return &g_stm32_di_list[num];
  }
  g_stm32_di_list[num].opened = true;

  switch(num)
  {
    case STM32_DI_ADC_RDY:
     g_stm32_di_list[num].cfg = (void *)&ADC_DRDY_cfg;
    stm32_di_init(&ADC_DRDY_cfg);
    break;
    case STM32_DI_RTC_IRQ:
    g_stm32_di_list[num].cfg = (void *)&RTC_IRQ_cfg;
    stm32_di_init(&RTC_IRQ_cfg);
    break;
    case STM32_DI_RAIN_REED:
    g_stm32_di_list[num].cfg = (void *)&RAIN_REED_cfg;
    stm32_di_init(&RAIN_REED_cfg);
    break;

    case STM32_DI_RAIN_HALL:
    g_stm32_di_list[num].cfg = (void *)&RAIN_HALL_cfg;
    stm32_di_init(&RAIN_HALL_cfg);
    break;

    case STM32_DI_RAIN_HALL_ERR:
    g_stm32_di_list[num].cfg = (void *)&RAIN_HALL_ERR_cfg;
    stm32_di_init(&RAIN_HALL_ERR_cfg);
    break;
    case STM32_DI_QUAD_UARTA_1:
    g_stm32_di_list[num].cfg = (void *)&QUAD_UARTA_1_cfg;
    stm32_di_init(&QUAD_UARTA_1_cfg);
    break;
    case STM32_DI_QUAD_UARTB_2:
    g_stm32_di_list[num].cfg = (void *)&QUAD_UARTB_2_cfg;
    stm32_di_init(&QUAD_UARTB_2_cfg);
    break;
    case STM32_DI_QUAD_UARTC_3:
    g_stm32_di_list[num].cfg = (void *)&QUAD_UARTC_3_cfg;
    stm32_di_init(&QUAD_UARTC_3_cfg);
    break;
    case STM32_DI_QUAD_UARTD_4:
    g_stm32_di_list[num].cfg = (void *)&QUAD_UARTD_4_cfg;
    stm32_di_init(&QUAD_UARTD_4_cfg);
    break;
    case STM32_DI_QUAD_UARTA_5:
    g_stm32_di_list[num].cfg = (void *)&QUAD_UARTA_5_cfg;
    stm32_di_init(&QUAD_UARTA_5_cfg);
    break;
    case STM32_DI_QUAD_UARTB_6:
    g_stm32_di_list[num].cfg = (void *)&QUAD_UARTB_6_cfg;
    stm32_di_init(&QUAD_UARTB_6_cfg);
    break;
    case STM32_DI_QUAD_UARTC_7:
    g_stm32_di_list[num].cfg = (void *)&QUAD_UARTC_7_cfg;
    stm32_di_init(&QUAD_UARTC_7_cfg);
    break;
    case STM32_DI_QUAD_UARTD_8:
    g_stm32_di_list[num].cfg = (void *)&QUAD_UARTD_8_cfg;
    stm32_di_init(&QUAD_UARTD_8_cfg);
    break;
  }
 
  return &g_stm32_di_list[num];
  
}

int32_t stm32_di_read(driver_t *driver)
{
  stm32_di_cfg_t *cfg = driver->cfg;

   return HAL_GPIO_ReadPin(cfg->port,cfg->pin);
}

void stm32_t_di_set(driver_t *drv,uint8_t cmd,void *option)
{
  switch (cmd)
  {
  case DI_SET_INTERRUT:
    break;
  
  default:
    break;
  }
}









#if 0 


typedef struct driver_ex_s
{
  const char *name;
  struct driver_ex_s *driver;
  void *cfg;
}driver_ex_t;
const driver_ex_t *stm32_open(const char *name)
{
  int cnt;
  gpio_cfg_t *cfg;

  cnt = _countof(g_gpio_list);

  for(int i=0;i<cnt;i++)
  {
    if(strncmp(name,g_gpio_list[i].name,strlen(name))==0)
    {
      cfg = (gpio_cfg_t *)g_gpio_list[i].cfg;
      if(cfg->opened==false)
      {
        cfg->opened = true;

      }

      return &g_gpio_list[i];
    }
  }

  return 0;
}
#endif


#include "stm32f4xx_hal.h"


IRQn_Type get_irqFromPin(uint16_t GPIO_Pin)
{

      IRQn_Type irq;
    if (GPIO_Pin == GPIO_PIN_0) {
        irq = EXTI0_IRQn;
    } else if (GPIO_Pin == GPIO_PIN_1) {
        irq = EXTI1_IRQn;
    } else if (GPIO_Pin == GPIO_PIN_2) {
        irq = EXTI2_IRQn;
    } else if (GPIO_Pin == GPIO_PIN_3) {
        irq = EXTI3_IRQn;
    } else if (GPIO_Pin == GPIO_PIN_4) {
        irq = EXTI4_IRQn;
    } else if (GPIO_Pin >= GPIO_PIN_5 && GPIO_Pin <= GPIO_PIN_9) {
        irq = EXTI9_5_IRQn;
    } else if (GPIO_Pin >= GPIO_PIN_10 && GPIO_Pin <= GPIO_PIN_15) {
        irq = EXTI15_10_IRQn;
    }

    return irq;
}

// GPIO 핀을 인터럽트 모드로 초기화하는 함수
void GPIO_InputInterrupt_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,  eDI_TRIGGER_t trigger,uint16_t prio)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 2. GPIO 핀 설정 (입력 모드, 풀업/풀다운)
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = eDI_RISING_FALLING; // 기본적으로 양 엣지로 설정
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    
    // 트리거 모드 설정 (Rising, Falling 또는 Both)
    if (trigger == eDI_RISING)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    }
    else if (trigger == eDI_FALLING)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    }
    else
    {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    }

    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);

    // 3. EXTI 인터럽트 우선순위 및 활성화 설정 (핀 번호에 따른 IRQ 설정)
    uint32_t exti_line = 0;
    IRQn_Type irq;

    irq = get_irqFromPin(GPIO_Pin);

    // 인터럽트 우선순위 설정 (우선순위 2, 하위 우선순위 0으로 설정)
    HAL_NVIC_SetPriority(irq, prio, 0);
    HAL_NVIC_EnableIRQ(irq);
}



void stm32_di_set(driver_t *drv,uint8_t cmd,void *option)
{
  stm32_di_cfg_t *cfg = drv->cfg;

  exti_isr_cfg_t exti_isr_cfg;

  switch(cmd)
  {
    case DI_SET_INTERRUT:
    {
      di_isr_set_cfg_t *isr;
      isr = (di_isr_set_cfg_t *)option;


      __HAL_GPIO_EXTI_CLEAR_IT(cfg->pin);
      exti_isr_cfg.irq =   get_irqFromPin(cfg->pin);
      exti_isr_cfg.call = isr->call;
      exti_isr_cfg.name = isr->name;
      exti_isr_cfg.gpio_pin = cfg->pin;
      exti_isr_cfg.handle = isr->handle;;
      exti_register(&exti_isr_cfg);

      GPIO_InputInterrupt_Init(cfg->port,cfg->pin,isr->trigger,isr->prio);

    }
    break;
  }
}