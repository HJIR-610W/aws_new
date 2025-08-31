

#include "driver_stm32_di.h"

#include "bsp.h"
#include "util_memory.h"
#include "pcb_define.h"

typedef struct stm32_di_inst_s
{
  GPIO_InitTypeDef init;
  GPIO_TypeDef *port;
  bool opened;
} stm32_di_inst_t;

#ifdef PCB_0_5 
stm32_di_inst_t di_inst[STM32_DI_MCU_MAX] = {

    [STM32_DI_0_ADC_RDY] = {.init = {.Pin = DI_SPI2_DRDY_Pin, .Pull = GPIO_PULLUP},
                          .port = DI_SPI2_DRDY_GPIO_Port},
    [STM32_DI_USER_BTN] = {.init = {.Pin = DI_SW_SYS_Pin, .Pull = GPIO_PULLUP},
                         .port = DI_SW_SYS_GPIO_Port},
    [STM32_DI_RAIN_REED] = {.init = {.Pin = DI_RAIN_REED_Pin, .Pull = GPIO_PULLUP},
                          .port = DI_RAIN_REED_GPIO_Port},
    [STM32_DI_RAIN_HALL] = {.init = {.Pin = DI_RAIN_HALL_Pin, .Pull = GPIO_PULLUP},
                          .port = DI_RAIN_HALL_GPIO_Port},
    [STM32_DI_RAIN_HALL_ERR] = {.init = {.Pin = DI_RAIN_HALL_ERR_Pin, .Pull = GPIO_PULLUP},
                              .port = DI_RAIN_HALL_ERR_GPIO_Port},
    [STM32_DI_RAIN_DETECT_A] = {.init = {.Pin = DI_RAIN_DETECT_Pin, .Pull = GPIO_PULLUP},
                              .port = DI_RAIN_DETECT_GPIO_Port},
    [STM32_DI_QUAD_UARTA_1] = {.init = {.Pin = DI_EX_UART_INT1_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT1_GPIO_Port},
    [STM32_DI_QUAD_UARTB_2] = {.init = {.Pin = DI_EX_UART_INT2_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT2_GPIO_Port},
    [STM32_DI_QUAD_UARTC_3] = {.init = {.Pin = DI_EX_UART_INT3_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT3_GPIO_Port},
    [STM32_DI_QUAD_UARTD_4] = {.init = {.Pin = DI_EX_UART_INT4_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT4_GPIO_Port},
    [STM32_DI_QUAD_UARTA_5] = {.init = {.Pin = DI_EX_UART_INT5_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT5_GPIO_Port},
    [STM32_DI_QUAD_UARTB_6] = {.init = {.Pin = DI_EX_UART_INT6_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT6_GPIO_Port},
    [STM32_DI_QUAD_UARTC_7] = {.init = {.Pin = DI_EX_UART_INT7_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT7_GPIO_Port},
    [STM32_DI_QUAD_UARTD_8] = {.init = {.Pin = DI_EX_UART_INT8_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT8_GPIO_Port},

    [STM32_DI_BOOT1] = {.init = {.Pin = DI_BOOT1_Pin, .Pull = GPIO_PULLUP},
                      .port = DI_BOOT1_GPIO_Port},
    [STM32_DI_USB_POWER_FAIL] = {.init = {.Pin = DI_USB_OTG_PWR_FAIL_Pin, .Pull = GPIO_PULLUP},
                               .port = DI_USB_OTG_PWR_FAIL_GPIO_Port},

    [STM32_DI_SD_IN] = {.init = {.Pin = DI_SDIO_DETECT_Pin, .Pull = GPIO_PULLUP},
                      .port = DI_SDIO_DETECT_GPIO_Port},

    [STM32_DI_IO_INT] = {.init = {.Pin = DI_INT_D_IO_Pin, .Pull = GPIO_PULLUP},
                       .port = DI_INT_D_IO_GPIO_Port},

    [STM32_DI_RTC_INT] = {.init = {.Pin = DI_INT_RTC_Pin, .Pull = GPIO_PULLUP},
                        .port = DI_INT_RTC_GPIO_Port},

    [STM32_DI_HART_CD] = {.init = {.Pin = DI_CD_H_Pin, .Pull = GPIO_PULLUP},
                        .port = DI_CD_H_GPIO_Port},
};
#endif


#ifdef PCB_0_6 
stm32_di_inst_t di_inst[STM32_DI_MCU_MAX] = {

    [STM32_DI_0_ADC_RDY] = {.init = {.Pin = DI_SPI2_DRDY_Pin, .Pull = GPIO_PULLUP},
                          .port = DI_SPI2_DRDY_GPIO_Port},
    [STM32_DI_USER_BTN] = {.init = {.Pin = DI_SW_SYS_Pin, .Pull = GPIO_PULLUP},
                         .port = DI_SW_SYS_GPIO_Port},
    [STM32_DI_RAIN_REED] = {.init = {.Pin = DI_RAIN_REED_Pin, .Pull = GPIO_PULLUP},
                          .port = DI_RAIN_REED_GPIO_Port},
    [STM32_DI_RAIN_HALL] = {.init = {.Pin = DI_RAIN_HALL_Pin, .Pull = GPIO_PULLUP},
                          .port = DI_RAIN_HALL_GPIO_Port},
    [STM32_DI_RAIN_HALL_ERR] = {.init = {.Pin = DI_RAIN_HALL_ERR_Pin, .Pull = GPIO_PULLUP},
                              .port = DI_RAIN_HALL_ERR_GPIO_Port},
    [STM32_DI_RAIN_DETECT_A] = {.init = {.Pin = DI_RAIN_DETECT_Pin, .Pull = GPIO_PULLUP},
                              .port = DI_RAIN_DETECT_GPIO_Port},
    [STM32_DI_QUAD_UARTA_1] = {.init = {.Pin = DI_EX_UART_INT1_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT1_GPIO_Port},
    [STM32_DI_QUAD_UARTB_2] = {.init = {.Pin = DI_EX_UART_INT2_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT2_GPIO_Port},
    [STM32_DI_QUAD_UARTC_3] = {.init = {.Pin = DI_EX_UART_INT3_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT3_GPIO_Port},
    [STM32_DI_QUAD_UARTD_4] = {.init = {.Pin = DI_EX_UART_INT4_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT4_GPIO_Port},
    [STM32_DI_QUAD_UARTA_5] = {.init = {.Pin = DI_EX_UART_INT5_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT5_GPIO_Port},
    [STM32_DI_QUAD_UARTB_6] = {.init = {.Pin = DI_EX_UART_INT6_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT6_GPIO_Port},
    [STM32_DI_QUAD_UARTC_7] = {.init = {.Pin = DI_EX_UART_INT7_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT7_GPIO_Port},
    [STM32_DI_QUAD_UARTD_8] = {.init = {.Pin = DI_EX_UART_INT8_Pin, .Pull = GPIO_PULLUP},
                             .port = DI_EX_UART_INT8_GPIO_Port},



    [STM32_DI_BOOT1] = {.init = {.Pin = DI_BOOT1_Pin, .Pull = GPIO_PULLUP},
                      .port = DI_BOOT1_GPIO_Port},
    [STM32_DI_USB_POWER_FAIL] = {.init = {.Pin = DI_USB_OTG_PWR_FAIL_Pin, .Pull = GPIO_PULLUP},
                               .port = DI_USB_OTG_PWR_FAIL_GPIO_Port},

    [STM32_DI_SD_IN] = {.init = {.Pin = DI_SDIO_DETECT_Pin, .Pull = GPIO_PULLUP},
                      .port = DI_SDIO_DETECT_GPIO_Port},

    [STM32_DI_IO_INT] = {.init = {.Pin = DI_INT_D_IO_Pin, .Pull = GPIO_PULLUP},
                       .port = DI_INT_D_IO_GPIO_Port},

    [STM32_DI_RTC_INT] = {.init = {.Pin = DI_INT_RTC_Pin, .Pull = GPIO_PULLUP},
                        .port = DI_INT_RTC_GPIO_Port},

    [STM32_DI_HART_CD] = {.init = {.Pin = DI_CD_H_Pin, .Pull = GPIO_PULLUP},
                        .port = DI_CD_H_GPIO_Port},
};
#endif


void stm32_di_gpio_init(int di_number)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  board_clk_gpio(di_inst[di_number].port);

  GPIO_InitStruct.Pin = di_inst[di_number].init.Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = di_inst[di_number].init.Pull;
  HAL_GPIO_Init(di_inst[di_number].port, &GPIO_InitStruct);
}

void stm32_di_init(void)
{
  for (int di_num = 0; di_num < STM32_DI_MCU_MAX; di_num++)
  {
    if (di_inst[di_num].opened)
      continue;

    di_inst[di_num].opened = true;
    switch (di_num)
    {
    case STM32_DI_0_ADC_RDY:
    case STM32_DI_USER_BTN:
    case STM32_DI_RAIN_REED:
    case STM32_DI_RAIN_HALL:
    case STM32_DI_RAIN_HALL_ERR:
    case STM32_DI_RAIN_DETECT_A:
    case STM32_DI_QUAD_UARTA_1:
    case STM32_DI_QUAD_UARTB_2:
    case STM32_DI_QUAD_UARTC_3:
    case STM32_DI_QUAD_UARTD_4:
    case STM32_DI_QUAD_UARTA_5:
    case STM32_DI_QUAD_UARTB_6:
    case STM32_DI_QUAD_UARTC_7:
    case STM32_DI_QUAD_UARTD_8:
      stm32_di_gpio_init(di_num);
      break;
    }
  }
}

int32_t stm32_di_read(int32_t di_number)
{
  switch (di_number)
  {
  case STM32_DI_0_ADC_RDY:
  case STM32_DI_USER_BTN:
  case STM32_DI_RAIN_REED:
  case STM32_DI_RAIN_HALL:
  case STM32_DI_RAIN_HALL_ERR:
  case STM32_DI_RAIN_DETECT_A:
  case STM32_DI_QUAD_UARTA_1:
  case STM32_DI_QUAD_UARTB_2:
  case STM32_DI_QUAD_UARTC_3:
  case STM32_DI_QUAD_UARTD_4:
  case STM32_DI_QUAD_UARTA_5:
  case STM32_DI_QUAD_UARTB_6:
  case STM32_DI_QUAD_UARTC_7:
  case STM32_DI_QUAD_UARTD_8:
    return HAL_GPIO_ReadPin(di_inst[di_number].port, di_inst[di_number].init.Pin);
    break;
  }
  return -1;
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
  out->Mode |= ((GPIOx->OTYPER >> pin_pos) & 0x1) << 4; // OpenDrain이면 OR로 표시 가능

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

  Read_GPIO_Config(GPIOx, GPIO_Pin, &GPIO_InitStruct);
  // 2. GPIO 핀 설정 (입력 모드, 풀업/풀다운)
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Mode = eDI_RISING_FALLING; // 기본적으로 양 엣지로 설정
  // GPIO_InitStruct.Pull = GPIO_NOPULL;

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

void stm32_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg)
{
  uint16_t pin;
  switch (di_number)
  {
  case STM32_DI_0_ADC_RDY:
  case STM32_DI_USER_BTN:
  case STM32_DI_RAIN_REED:
  case STM32_DI_RAIN_HALL:
  case STM32_DI_RAIN_HALL_ERR:
  case STM32_DI_RAIN_DETECT_A:
  case STM32_DI_QUAD_UARTA_1:
  case STM32_DI_QUAD_UARTB_2:
  case STM32_DI_QUAD_UARTC_3:
  case STM32_DI_QUAD_UARTD_4:
  case STM32_DI_QUAD_UARTA_5:
  case STM32_DI_QUAD_UARTB_6:
  case STM32_DI_QUAD_UARTC_7:
  case STM32_DI_QUAD_UARTD_8:
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

    GPIO_InputInterrupt_Init(di_inst[di_number].port, pin, isr->trigger, isr->prio);
  }
  }
}