



#include "driver_digitalOut.h"

#include "stm32f4xx_hal.h"
#include "main.h"
typedef struct digitalOut_cfg_S
{
    GPIO_TypeDef *gpio;
    GPIO_PinState pin;
   
}digitalOut_cfg_t;


typedef struct digitalOut_api_s
{
  void (*low)(digitalOut_cfg_t *);
  void (*high)(digitalOut_cfg_t *);
}digitalOut_api_t;


const digitalOut_cfg_t g_CON_PWR_CDMA_cfg = {.gpio = OUT_CON_PWR_CDMA_GPIO_Port,.pin=OUT_CON_PWR_CDMA_Pin};
const digitalOut_cfg_t g_ADC_NCS_cfg = {     .gpio = OUT_SPI2_NSS_GPIO_Port,    .pin=OUT_SPI2_NSS_Pin};
const digitalOut_cfg_t g_FRAM_CS_cfg = {     .gpio = OUT_FLASH_CS_GPIO_Port,    .pin=OUT_FLASH_CS_Pin};


//const digitalOut_cfg_t g_FLASH_CS_cfg={     .gpio=OUT_SPI2_NSS_GPIO_Port,    .pin=OUT_SPI2_NSS_Pin};
//const digitalOut_cfg_t g_RTC_CS_cfg={     .gpio=OUT_SPI2_NSS_GPIO_Port,    .pin=OUT_SPI2_NSS_Pin};


static const digitalOut_api_t g_pwrCDMA_api;



void digitalOut_low(digitalOut_cfg_t *cfg)
{
    HAL_GPIO_WritePin(cfg->gpio,cfg->pin,GPIO_PIN_RESET);
}


void digitalOut_high(digitalOut_cfg_t *cfg)
{
    HAL_GPIO_WritePin(cfg->gpio,cfg->pin,GPIO_PIN_SET);
}





void driver_digitalOut_init(driver_digitalOut_t *digitalOut,uint32_t num)
{
      GPIO_InitTypeDef GPIO_InitStruct = {0};

  switch(num)
  {
    case CON_PWR_CDMA:
      digitalOut->apiCfg = (void *)&g_CON_PWR_CDMA_cfg;
      digitalOut->api = (void *)&g_pwrCDMA_api;
      break;
    case DO_ADC_NCS:
      digitalOut->api = (void *)&g_pwrCDMA_api;
      digitalOut->apiCfg = (void *)&g_ADC_NCS_cfg;

      GPIO_InitStruct.Pin = OUT_SPI2_NSS_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(OUT_SPI2_NSS_GPIO_Port, &GPIO_InitStruct);

    break;


    
  }
}


void driver_digitalOut_low(driver_digitalOut_t *digitalOut)
{
    ((digitalOut_api_t *)digitalOut->api)->low(digitalOut->apiCfg);
}

void driver_digitalOut_high(driver_digitalOut_t *digitalOut)
{
    ((digitalOut_api_t *)digitalOut->api)->high(digitalOut->apiCfg);
}



static const digitalOut_api_t g_pwrCDMA_api={.low = digitalOut_low,
                                         .high = digitalOut_high
};


//STM32 GPIO 파일 시작

#define GPIO_DO_SET 0x01
typedef struct stm32_gpio_cfg_s
{
 GPIO_TypeDef *port;
 uint16_t pin;

}stm32_gpio_cfg_t;


driver_t *stm32_do_open(int num)
{
  switch(num)
  {
    case DO_FRAM_CS:
      static driver_t stm32_do;
      static stm32_gpio_cfg_t cfg;

      stm32_do.cfg =&cfg;

      return &stm32_do;
      break;
    case DO_RTC_CS:
    static driver_t rtc_cs;
    static stm32_gpio_cfg_t rtc_cfg;

      rtc_cs.cfg =&rtc_cfg;

      return &rtc_cs;
      break;
    break;
  }
}


void do_low(driver_t *driver)
{
  stm32_gpio_cfg_t *cfg = driver->cfg;

  HAL_GPIO_WritePin(cfg->port,cfg->pin,GPIO_PIN_RESET);
}

//

void do_high(driver_t *driver)
{
  stm32_gpio_cfg_t *cfg = driver->cfg;

  HAL_GPIO_WritePin(cfg->port,cfg->pin,GPIO_PIN_SET);
}


void do_set(driver_t *driver,int cmd,void *cfg)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
  switch (cmd)
  {
  case GPIO_DO_SET:
    GPIO_InitStruct.Pin = ((stm32_gpio_cfg_t *)cfg)->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(((stm32_gpio_cfg_t *)cfg)->port, &GPIO_InitStruct);
    break;
  
  default:
    break;
  }
}


//stm32 파일끝

//driver_digitalOut.c 시작
typedef struct do_api_s
{
  void (*low)(driver_t *driver);
  void (*high)(driver_t *driver);
}fram_api_t;

driver_t *driver_do_open(int num)
{
    static fram_api_t fram_api={.high =do_high,
                         .low = do_low};
  switch(num)
  {
    case DO_FRAM_CS:
    static driver_t fram_cs;
    static driver_t *p_stm32_do;
    stm32_gpio_cfg_t *p_stm32_gpio_cfg;


    p_stm32_do = stm32_do_open(num);
    p_stm32_gpio_cfg = (stm32_gpio_cfg_t*)p_stm32_do->cfg;
    p_stm32_gpio_cfg->port = OUT_SPI1_NSS_GPIO_Port;
    p_stm32_gpio_cfg->pin  = OUT_SPI1_NSS_Pin;

    do_set(p_stm32_do,GPIO_DO_SET,p_stm32_gpio_cfg);

    fram_cs.api    = &fram_api;//호출시 사용될 API
    fram_cs.handle = p_stm32_do;//연결된 IC

    return &fram_cs;
    break;
    case DO_RTC_CS:
    static driver_t rtc_cs;// driver_rtc에 대한 것
    static driver_t *p_stm_rtc_cs;//stm32_rtc_cs에 대한것
    stm32_gpio_cfg_t *p_stm32_rtc_cfg;

    p_stm_rtc_cs = stm32_do_open(num);
    p_stm32_rtc_cfg = (stm32_gpio_cfg_t*)p_stm_rtc_cs->cfg;
    p_stm32_rtc_cfg->port = OUT_SPI1_CS_RTC_GPIO_Port;
    p_stm32_rtc_cfg->pin  = OUT_SPI1_CS_RTC_Pin;


    do_set(p_stm32_do,GPIO_DO_SET,p_stm32_rtc_cfg);

    rtc_cs.api    = &fram_api;//호출시 사용될 API
    rtc_cs.handle = p_stm_rtc_cs;//연결된 IC

    return &rtc_cs;



    break;

  }

  return 0;
}

void driver_do_low(driver_t *digitalOut)
{
   const fram_api_t *api =digitalOut->api;
     api->low(digitalOut->handle);
}

void driver_do_high(driver_t *digitalOut)
{
     const fram_api_t *api =digitalOut->api;
     api->high(digitalOut->handle);

   // ((digitalOut_api_t *)digitalOut->api)->high(digitalOut->apiCfg);
}