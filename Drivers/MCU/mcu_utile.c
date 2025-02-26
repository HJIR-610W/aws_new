

#include "mcu_utile.h"


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
