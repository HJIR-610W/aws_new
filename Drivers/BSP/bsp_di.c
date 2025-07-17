

#include "bsp_di.h"

#include "bsp.h"
#include "pcf8575.h"
#include "util_memory.h"
#include "pcb_define.h"


typedef struct bsp_di_inst_s
{
  GPIO_InitTypeDef init;
  GPIO_TypeDef *port;
  bool opened;
} bsp_di_inst_t;

const bsp_di_inst_t di_inst[BSP_DI_QUAD_UARTD_8 + 1] = {

    [BSP_DI_0_ADC_RDY_ONLY] = {.init = {.Pin = IN_SPI2_DRDY_Pin, .Pull = GPIO_PULLUP},
                          .port = IN_SPI2_DRDY_GPIO_Port},
    [BSP_DI_USER_BTN] = {.init = {.Pin = DI_SW_SYS_Pin, .Pull = GPIO_PULLUP},
                         .port = DI_SW_SYS_GPIO_Port},
    [BSP_DI_RAIN_REED] = {.init = {.Pin = IN_RAIN_REED_Pin, .Pull = GPIO_PULLUP},
                          .port = IN_RAIN_REED_GPIO_Port},
    [BSP_DI_RAIN_HALL] = {.init = {.Pin = IN_RAIN_HALL_Pin, .Pull = GPIO_PULLUP},
                          .port = IN_RAIN_HALL_GPIO_Port},
    [BSP_DI_RAIN_HALL_ERR] = {.init = {.Pin = IN_RAIN_HALL_ERR_Pin, .Pull = GPIO_PULLUP},
                              .port = IN_RAIN_HALL_ERR_GPIO_Port},
    [BSP_DI_RAIN_DETECT] = {.init = {.Pin = RAIN_DETECT_PIN, .Pull = GPIO_PULLUP},
                            .port = RAIN_DETECT_GPIO_Port},
    [BSP_DI_QUAD_UARTA_1] = {.init = {.Pin = IN_EX_UART_INT_1_Pin, .Pull = GPIO_PULLUP},
                             .port = IN_EX_UART_INT_1_GPIO_Port},
    [BSP_DI_QUAD_UARTB_2] = {.init = {.Pin = IN_EX_UART_INT_2_Pin, .Pull = GPIO_PULLUP},
                             .port = IN_EX_UART_INT_2_GPIO_Port},
    [BSP_DI_QUAD_UARTC_3] = {.init = {.Pin = IN_EX_UART_INT_3_Pin, .Pull = GPIO_PULLUP},
                             .port = IN_EX_UART_INT_3_GPIO_Port},
    [BSP_DI_QUAD_UARTD_4] = {.init = {.Pin = IN_EX_UART_INT_4_Pin, .Pull = GPIO_PULLUP},
                             .port = IN_EX_UART_INT_4_GPIO_Port},
    [BSP_DI_QUAD_UARTA_5] = {.init = {.Pin = IN_EX_UART_INT_5_Pin, .Pull = GPIO_PULLUP},
                             .port = IN_EX_UART_INT_5_GPIO_Port},
    [BSP_DI_QUAD_UARTB_6] = {.init = {.Pin = IN_EX_UART_INT_6_Pin, .Pull = GPIO_PULLUP},
                             .port = IN_EX_UART_INT_6_GPIO_Port},
    [BSP_DI_QUAD_UARTC_7] = {.init = {.Pin = IN_EX_UART_INT_7_Pin, .Pull = GPIO_PULLUP},
                             .port = IN_EX_UART_INT_7_GPIO_Port},
    [BSP_DI_QUAD_UARTD_8] = {.init = {.Pin = IN_EX_UART_INT_8_Pin, .Pull = GPIO_PULLUP},
                             .port = IN_EX_UART_INT_8_GPIO_Port}};

void bsp_di_gpio_init(int di_number)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  board_clk_gpio(di_inst[di_number].port);

  GPIO_InitStruct.Pin = di_inst[di_number].init.Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = di_inst[di_number].init.Pull;
  HAL_GPIO_Init(di_inst[di_number].port, &GPIO_InitStruct);
}

void bsp_di_init(void)
{
  for (int di_num = 0; di_num < BSP_DI_MCU_MAX; di_num++)
  {
    if (di_inst[di_num].opened)
    continue;
    
      switch (di_num)
      {
        case BSP_DI_0_ADC_RDY_ONLY:
        case BSP_DI_USER_BTN:
        case BSP_DI_RAIN_REED:
        case BSP_DI_RAIN_HALL:
        case BSP_DI_RAIN_HALL_ERR:
        case BSP_DI_RAIN_DETECT:
        case BSP_DI_QUAD_UARTA_1:
        case BSP_DI_QUAD_UARTB_2:
        case BSP_DI_QUAD_UARTC_3:
        case BSP_DI_QUAD_UARTD_4:
        case BSP_DI_QUAD_UARTA_5:
        case BSP_DI_QUAD_UARTB_6:
        case BSP_DI_QUAD_UARTC_7:
        case BSP_DI_QUAD_UARTD_8:
          bsp_di_gpio_init(di_num);
          break;
        case BSP_DI_0:
        case BSP_DI_1:
        case BSP_DI_2:
        case BSP_DI_3:
        case BSP_DI_4:
        case BSP_DI_5:
        case BSP_DI_6:
        case BSP_DI_7:
          pcf8575_init();
          break;

        default:
          break;
      }
  }
}

int32_t bsp_di_read(int32_t di_number)
{
  switch (di_number)
  {
    case BSP_DI_0_ADC_RDY_ONLY:
    case BSP_DI_USER_BTN:
    case BSP_DI_RAIN_REED:
    case BSP_DI_RAIN_HALL:
    case BSP_DI_RAIN_HALL_ERR:
    case BSP_DI_RAIN_DETECT:
    case BSP_DI_QUAD_UARTA_1:
    case BSP_DI_QUAD_UARTB_2:
    case BSP_DI_QUAD_UARTC_3:
    case BSP_DI_QUAD_UARTD_4:
    case BSP_DI_QUAD_UARTA_5:
    case BSP_DI_QUAD_UARTB_6:
    case BSP_DI_QUAD_UARTC_7:
    case BSP_DI_QUAD_UARTD_8:
      return HAL_GPIO_ReadPin(di_inst[di_number].port, di_inst[di_number].init.Pin);
      break;
    case BSP_DI_0:
      return pcf8575_read_pin(DI_PCF8575_0);
      break; 
    case BSP_DI_1:
      return pcf8575_read_pin(DI_PCF8575_1);
      break;
    case BSP_DI_2:
      return pcf8575_read_pin(DI_PCF8575_2);
      break;
    case BSP_DI_3:
      return pcf8575_read_pin(DI_PCF8575_3);
      break;
    case BSP_DI_4:
      return pcf8575_read_pin(DI_PCF8575_4);
      break;
    case BSP_DI_5:
      return pcf8575_read_pin(DI_PCF8575_5);
      break;
    case BSP_DI_6:
      return pcf8575_read_pin(DI_PCF8575_6);
      break;
    case BSP_DI_7:
      return pcf8575_read_pin(DI_PCF8575_7);
      break;
    default:
      break;
  }
  return -99;
}


IRQn_Type get_irqFromPin(uint16_t GPIO_Pin)
{
  IRQn_Type irq;
  if (GPIO_Pin == GPIO_PIN_0)
  {
    irq = EXTI0_IRQn;
  }
  else if (GPIO_Pin == GPIO_PIN_1)
  {
    irq = EXTI1_IRQn;
  }
  else if (GPIO_Pin == GPIO_PIN_2)
  {
    irq = EXTI2_IRQn;
  }
  else if (GPIO_Pin == GPIO_PIN_3)
  {
    irq = EXTI3_IRQn;
  }
  else if (GPIO_Pin == GPIO_PIN_4)
  {
    irq = EXTI4_IRQn;
  }
  else if (GPIO_Pin >= GPIO_PIN_5 && GPIO_Pin <= GPIO_PIN_9)
  {
    irq = EXTI9_5_IRQn;
  }
  else if (GPIO_Pin >= GPIO_PIN_10 && GPIO_Pin <= GPIO_PIN_15)
  {
    irq = EXTI15_10_IRQn;
  }

  return irq;
}


void Read_GPIO_Config(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_InitTypeDef *out)
{
  uint32_t pin_pos = 0;

  // 핀 위치 계산 (0~15)
  for (pin_pos = 0; pin_pos < 16; pin_pos++)
  {
    if ((GPIO_Pin >> pin_pos) & 0x1)
      break;
  }

  // MODER (2비트 당 1핀)
  out->Mode = (GPIOx->MODER >> (pin_pos * 2)) & 0x3;

  // OTYPER (1비트 당 1핀)
  out->Mode |= ((GPIOx->OTYPER >> pin_pos) & 0x1) << 4;  // OpenDrain이면 OR로 표시 가능

  // OSPEEDR (2비트 당 1핀)
  out->Speed = (GPIOx->OSPEEDR >> (pin_pos * 2)) & 0x3;

  // PUPDR (2비트 당 1핀)
  out->Pull = (GPIOx->PUPDR >> (pin_pos * 2)) & 0x3;

  // AFR[0] for pin 0~7, AFR[1] for pin 8~15
  if (pin_pos < 8)
  {
    out->Alternate = (GPIOx->AFR[0] >> (pin_pos * 4)) & 0xF;
  }
  else
  {
    out->Alternate = (GPIOx->AFR[1] >> ((pin_pos - 8) * 4)) & 0xF;
  }

  // Pin 정보 그대로 저장
  out->Pin = GPIO_Pin;
}


// GPIO 핀을 인터럽트 모드로 초기화하는 함수
void GPIO_InputInterrupt_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                              eDI_TRIGGER_t trigger, uint16_t prio)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  
  Read_GPIO_Config(GPIOx,GPIO_Pin,&GPIO_InitStruct);
  // 2. GPIO 핀 설정 (입력 모드, 풀업/풀다운)
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Mode = eDI_RISING_FALLING;  // 기본적으로 양 엣지로 설정
  //GPIO_InitStruct.Pull = GPIO_NOPULL;

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

  IRQn_Type irq;

  irq = get_irqFromPin(GPIO_Pin);

  // 인터럽트 우선순위 설정 (우선순위 2, 하위 우선순위 0으로 설정)
  HAL_NVIC_SetPriority(irq, prio, 0);
  HAL_NVIC_EnableIRQ(irq);
}
void bsp_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg)
{
  uint16_t pin;
  switch (di_number)
  {
    case BSP_DI_0_ADC_RDY_ONLY:
    case BSP_DI_USER_BTN:
    case BSP_DI_RAIN_REED:
    case BSP_DI_RAIN_HALL:
    case BSP_DI_RAIN_HALL_ERR:
    case BSP_DI_RAIN_DETECT:
    case BSP_DI_QUAD_UARTA_1:
    case BSP_DI_QUAD_UARTB_2:
    case BSP_DI_QUAD_UARTC_3:
    case BSP_DI_QUAD_UARTD_4:
    case BSP_DI_QUAD_UARTA_5:
    case BSP_DI_QUAD_UARTB_6:
    case BSP_DI_QUAD_UARTC_7:
    case BSP_DI_QUAD_UARTD_8:
    {
      exti_isr_cfg_t exti_isr_cfg;
      di_isr_set_cfg_t *isr;
      isr = (di_isr_set_cfg_t *)isr_cfg;
      pin = di_inst[di_number].init.Pin;

      __HAL_GPIO_EXTI_CLEAR_IT(pin);
      exti_isr_cfg.irq = get_irqFromPin(pin);
      exti_isr_cfg.call = isr->call;
      exti_isr_cfg.name = isr->name;
      exti_isr_cfg.gpio_pin = pin;
      exti_isr_cfg.handle = isr->handle;
      exti_register(&exti_isr_cfg);

      GPIO_InputInterrupt_Init(di_inst[di_number].port,pin, isr->trigger, isr->prio);
      }
    }
 


}