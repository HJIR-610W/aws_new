



#include "driver_digitalIn.h"

#include "stm32f4xx_hal.h"
#include "main.h"
#include "mcu_interrupt.h"
typedef struct digitalIn_cfg_S
{
    GPIO_TypeDef *gpio;
    GPIO_PinState pin;
   
}digitalIn_cfg_t;


typedef struct digitalOut_api_s
{
  uint32_t (*read)(digitalIn_cfg_t *);
}digitalIn_api_t;


const digitalIn_cfg_t g_ADC_DRY_cfg = {     .gpio = IN_SPI2_DRDY_GPIO_Port,    .pin=IN_SPI2_DRDY_Pin};


static const digitalIn_api_t g_ADC_DRY_api;



uint32_t digitalIn_read(digitalIn_cfg_t *cfg)
{
   return HAL_GPIO_ReadPin(cfg->gpio,cfg->pin);
}






void driver_digitalIn_init(driver_digitalIn_t *digitalIn,uint32_t num)
{
      GPIO_InitTypeDef GPIO_InitStruct = {0};

  switch(num)
  {
    case DI_ADC_RDY:
      digitalIn->apiCfg = (void *)&g_ADC_DRY_cfg;
      digitalIn->api = (void *)&g_ADC_DRY_api;

    GPIO_InitStruct.Pin = IN_SPI2_DRDY_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(IN_SPI2_DRDY_GPIO_Port, &GPIO_InitStruct);
      break;
  }
}


uint32_t driver_digitalIn_read(driver_digitalIn_t *digitalOut)
{
  uint32_t v;
  v =   ((digitalIn_api_t *)digitalOut->api)->read(digitalOut->apiCfg);
  return v;
}


void driver_digitalIn_set(driver_digitalIn_t *digitalIn,uint32_t cmd,void *data)
{


  switch(cmd)
  {
    case eDIG_IN_SET_INTERRUPT:
    digitalIn_set_interupt_t *isr = (digitalIn_set_interupt_t *)data;
    exti_register(((digitalIn_cfg_t *)digitalIn->apiCfg)->pin,isr->handle,isr->call);
    
    break;
  }
}




static const digitalIn_api_t g_ADC_DRY_api={.read = digitalIn_read};






//STM32 GPIO 파일 시작

#define GPIO_DI_SET 0x01
typedef struct stm32_gpio_cfg_s
{
 GPIO_TypeDef *port;
 uint16_t pin;

}stm32_gpio_cfg_t;


driver_t *stm32_di_open(int num)
{
  switch(num)
  {
    case DI_RTC_IRQ:
      static driver_t stm32_do;
      static stm32_gpio_cfg_t cfg;

      stm32_do.cfg =&cfg;

      return &stm32_do;
  }
}


uint16_t di_read(driver_t *driver)
{
  stm32_gpio_cfg_t *cfg = driver->cfg;

  return (uint16_t )HAL_GPIO_ReadPin(cfg->port,cfg->pin);
}



void di_set(driver_t *driver,int cmd,void *cfg)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
  switch (cmd)
  {
  case GPIO_DI_SET:
    GPIO_InitStruct.Pin = ((stm32_gpio_cfg_t *)cfg)->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(((stm32_gpio_cfg_t *)cfg)->port, &GPIO_InitStruct);
    break;
  
  default:
    break;
  }
}


//stm32 파일끝

//driver_digitalOut.c 시작
typedef struct di_api_s
{
  uint16_t (*read)(driver_t *driver);
}di_api_t;

driver_t *driver_di_open(int num)
{

  switch(num)
  {
    case DI_RTC_IRQ:
    static driver_t rtc_irq;
    static driver_t *p_stm32_di;
    stm32_gpio_cfg_t *p_stm32_gpio_cfg;
    static di_api_t di_api={.read = di_read};

    p_stm32_di = stm32_di_open(num);
    p_stm32_gpio_cfg = (stm32_gpio_cfg_t*)p_stm32_di->cfg;
    p_stm32_gpio_cfg->port = INT_RTC_GPIO_Port;
    p_stm32_gpio_cfg->pin  = INT_RTC_Pin;

    di_set(p_stm32_di,GPIO_DI_SET,p_stm32_gpio_cfg);

    rtc_irq.api    = &di_api;//호출시 사용될 API
    rtc_irq.handle = p_stm32_di;//연결된 IC

    return &rtc_irq;
    break;

  }

  return 0;
}

uint16_t driver_di_read(driver_t *driver)
{
     const di_api_t *api = driver->api;

    uint16_t pin;

    pin =  api->read(driver->handle);

    return pin;
}

