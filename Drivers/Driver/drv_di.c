
#include "bsp.h"

#include "drv_di.h"
#include "bsp_di.h"


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

void drv_di_init(void)
{
  bsp_di_init();
}


int32_t drv_di_read(int di_number)
{
  switch (di_number)
  {
    case DRV_DI_0:
      return bsp_di_read(BSP_DI_0);
    case DRV_DI_1:
      return bsp_di_read(BSP_DI_1);
    case DRV_DI_2:
      return bsp_di_read(BSP_DI_2);
    case DRV_DI_3:
      return bsp_di_read(BSP_DI_3);
    case DRV_DI_4:
      return bsp_di_read(BSP_DI_4);
    case DRV_DI_5:
      return bsp_di_read(BSP_DI_5);
    case DRV_DI_6:
      return bsp_di_read(BSP_DI_6);
    case DRV_DI_7:
      return bsp_di_read(BSP_DI_7);
    case DI_RAIN_REED:
      return bsp_di_read(BSP_DI_RAIN_REED);
    case DI_RAIN_HALL:
      return bsp_di_read(BSP_DI_RAIN_HALL);
    case DI_RAIN_HALL_ERR:
      return bsp_di_read(BSP_DI_RAIN_HALL_ERR);
    case DI_USER_BTN:
      return bsp_di_read(BSP_DI_USER_BTN);
    case DI_RAIN_DETECT:
      return bsp_di_read(BSP_DI_RAIN_DETECT);
    default:
    {
      return -1;
    }
  }

  return -1;
}

void  drv_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg)
{

}