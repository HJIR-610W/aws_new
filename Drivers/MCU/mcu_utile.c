

#include "mcu_utile.h"


/*
GPIOx의 클럭이 enable 안되어 있으면 enable 해줌
*/
void board_clk_gpio(GPIO_TypeDef *GPIOx)
{
  uint32_t port;

  port = (uint32_t)GPIOx;

  switch(port)
  {
    case (uint32_t)GPIOA:
    if(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN)==0)
    {
      __HAL_RCC_GPIOA_CLK_ENABLE();
    }
      break;
          case (uint32_t)GPIOB:
    if(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN)==0)
    {
      __HAL_RCC_GPIOB_CLK_ENABLE();
    }
      break;
          case (uint32_t)GPIOC:
    if(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN)==0)
    {
      __HAL_RCC_GPIOC_CLK_ENABLE();
    }
      break;
          case (uint32_t)GPIOD:
    if(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN)==0)
    {
      __HAL_RCC_GPIOD_CLK_ENABLE();
    }
      break;
          case (uint32_t)GPIOE:
    if(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOEEN)==0)
    {
      __HAL_RCC_GPIOE_CLK_ENABLE();
    }
      break;
          case (uint32_t)GPIOH:
    if(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOHEN)==0)
    {
      __HAL_RCC_GPIOH_CLK_ENABLE();
    }
      break;
          case (uint32_t)GPIOF:
    if(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOFEN)==0)
    {
      __HAL_RCC_GPIOF_CLK_ENABLE();
    }
      break;
          case (uint32_t)GPIOG:
    if(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOGEN)==0)
    {
      __HAL_RCC_GPIOG_CLK_ENABLE();
    }
      break;
  }

}
