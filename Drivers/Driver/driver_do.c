
#include "driver_do.h"
#include "driver_do_define.h"
#include "driver_stm32_do.h"
#include "pcf8575.h"


driver_t *driver_do_open(uint32_t num,void *opt)
{
  driver_t *driver = 0;

  switch(num)
  {
    case DO_PWR_CDMA:
    driver = stm32_do_open(STM32_DO_PWR_CDMA,opt);
    break;
    case DO_ADC_NCS:
    driver = stm32_do_open(STM32_DO_ADC_NCS,opt);
    break;
        case DO_FRAM_CS:
    driver = stm32_do_open(STM32_DO_FRAM_CS,opt);
    break;
        case DO_RTC_CS:
    driver = stm32_do_open(STM32_DO_RTC_CS,opt);
    break;
        case DO_FLASH_CS:
    driver = stm32_do_open(STM32_DO_FLASH_CS,opt);
    break;
        case DO_DIR_SDI:
    driver = stm32_do_open(STM32_DO_DIR_SDI,opt);
    break;
        case DO_DIR_RS485_A:
    driver = stm32_do_open(STM32_DO_DIR_RS485_A,opt);
    break;
        case DO_DIR_RS485_B:
    driver = stm32_do_open(STM32_DO_DIR_RS485_B,opt);
    break;
    case DO_EXT_0:
    driver = pcf8575_do_open(DO_PCF8575_0,0);
    break;
    case DO_EXT_1:
    driver = pcf8575_do_open(DO_PCF8575_1,0);
    break;
    case DO_EXT_2:
    driver = pcf8575_do_open(DO_PCF8575_2,0);
    break;
    case DO_EXT_3:
    driver = pcf8575_do_open(DO_PCF8575_3,0);
    break;
    case DO_EXT_4:
    driver = pcf8575_do_open(DO_PCF8575_4,0);
    break;
    case DO_EXT_5:
    driver = pcf8575_do_open(DO_PCF8575_5,0);
    break;
    case DO_HART_RTS:
    driver = stm32_do_open(STM32_DO_HART_RTS,opt);
    break;
    case DO_HART_SEL:
    driver = stm32_do_open(STM32_DO_HART_SEL,opt);
    break;
    case DO_POWER_HART_24V_ACTIVE_H:
    driver = stm32_do_open(STM32_DO_POWER_24V,opt);
    break;
    case DO_HART_RESET:
    driver = stm32_do_open(STM32_DO_HART_RESET,opt);
    break;
    case DO_BTM_PWCTRL:
      driver = stm32_do_open(STM32_DO_BTM_PWRC,opt);
      break;
    case DO_DIR_RS485_C:
      driver = stm32_do_open(STM32_DO_DIR_RS485_C, opt);
      break;
    case DO_DIR_RS485_D:
      driver = stm32_do_open(STM32_DO_DIR_RS485_D, opt);
      break;
    case DO_CON_PWR_RAIN_DECT_ACTIVE_H:
      driver = stm32_do_open(STM32_DO_CON_PWR_RAIN_DECT_ACTIVE_H, opt);
      break;
  }

  return driver;
}


void driver_do_low(driver_t *drv)
{
  const do_api_t *api = drv->api;

  api->low(drv);

}

void driver_do_high(driver_t *drv)
{
  const do_api_t *api = drv->api;

  api->high(drv);
}

void driver_do_set(driver_t *drv, do_set_option_t option, void *value)
{


}

void driver_do_close(driver_t *drv)
{

}




