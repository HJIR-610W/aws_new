
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
