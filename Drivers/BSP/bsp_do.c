
#include "bsp_do.h"
#include "bsp.h"

typedef struct mcu_gpio_instance_s
{
  GPIO_InitTypeDef gpio;
  GPIO_TypeDef *gpio_hanle;
} mcu_gpio_instance_t;

const GPIO_InitTypeDef gpio_default;





    void  bsp_cdma_power_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  board_clk_gpio(OUT_PWR_CDMA_GPIO_Port);
  GPIO_InitStruct.Pin = OUT_PWR_CDMA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(OUT_PWR_CDMA_GPIO_Port, &GPIO_InitStruct);
}

void bsp_do_init(void)
{

      bsp_cdma_power_init();
   
}

#define BSP_DO_CDMA_POWER_LOW() HAL_GPIO_WritePin(OUT_PWR_CDMA_GPIO_Port, OUT_PWR_CDMA_PIN,GPIO_PIN_RESET)
#define BSP_DO_CDMA_POWER_HIGH() \
  HAL_GPIO_WritePin(OUT_PWR_CDMA_GPIO_Port, OUT_PWR_CDMA_PIN, GPIO_PIN_RESET)


void bsp_do_low(int num)
{
  switch (num)
  {
    case BSP_CDMA_POWER:
      BSP_DO_CDMA_POWER_LOW();
      break;

    default:
      break;
  }
}

void bsp_do_high(int num)
{
  switch (num)
  {
    case BSP_CDMA_POWER:

      break;

    default:
      break;
  }
}
