
#include "cmsis_os.h"
#include "driver_di.h"
#include "driver_stm32_di.h"
#include "pcf8575.h"
driver_t *driver_di_open(uint32_t num,void *opt)
{
  driver_t *driver = NULL;

  switch (num)
  {
    case DI_0_ADC_RDY:  
    driver = stm32_di_open(STM32_DI_0_ADC_RDY,opt);
    break;     
    case DI_1_RTC_IRQ:    
      driver = stm32_di_open(STM32_DI_1_RTC_IRQ,opt);
    break;     
    case DI_RAIN_REED:
      driver = stm32_di_open(STM32_DI_RAIN_REED,opt);
    break;       
    case DI_RAIN_HALL:
      driver = stm32_di_open(STM32_DI_RAIN_HALL,opt);
    break;       
    case DI_RAIN_HALL_ERR:
      driver = stm32_di_open(STM32_DI_RAIN_HALL_ERR,opt);
    break;   
    case DI_QUAD_UARTA_1: 
      driver = stm32_di_open(STM32_DI_QUAD_UARTA_1,opt);
    break;   
    case DI_QUAD_UARTB_2:
      driver = stm32_di_open(STM32_DI_QUAD_UARTB_2,opt);
    break;    
    case DI_QUAD_UARTC_3:
      driver = stm32_di_open(STM32_DI_QUAD_UARTC_3,opt);
    break;    
    case DI_QUAD_UARTD_4:
      driver = stm32_di_open(STM32_DI_QUAD_UARTD_4,opt);
    break;    
    case DI_QUAD_UARTA_5:
      driver = stm32_di_open(STM32_DI_QUAD_UARTA_5,opt);
    break;   
    case DI_QUAD_UARTB_6:
      driver = stm32_di_open(STM32_DI_QUAD_UARTB_6,opt);
    break;   
    case DI_QUAD_UARTC_7:
      driver = stm32_di_open(STM32_DI_QUAD_UARTC_7,opt);
    break;   
    case DI_QUAD_UARTD_8: 
      driver = stm32_di_open(STM32_DI_QUAD_UARTD_8,opt);
    break;  
    case DI_EXT_0:
    driver = pcf8575_di_open(DI_PCF8575_0,0);
    break;
    case DI_EXT_1:
    driver = pcf8575_di_open(DI_PCF8575_1,0);
    break;
    case DI_EXT_2:
    driver = pcf8575_di_open(DI_PCF8575_2,0);
    break;
    case DI_EXT_3:
    driver = pcf8575_di_open(DI_PCF8575_3,0);
    break;
    case DI_EXT_4:
    driver = pcf8575_di_open(DI_PCF8575_4,0);
    break;
    case DI_EXT_5:
    driver = pcf8575_di_open(DI_PCF8575_5,0);
    break;
    case DI_EXT_6:
    driver = pcf8575_di_open(DI_PCF8575_6,0);
    break;
    case DI_EXT_7:
    driver = pcf8575_di_open(DI_PCF8575_7,0);
    break;
    case DI_HART_CD:
    driver = stm32_di_open(STM32_DI_HART_CD,opt);
    break;
    case DI_USER_BTN:
    driver = stm32_di_open(STM32_DI_USER_BTN,opt);
    break;
    case DI_BTM_STATUS:
    driver = stm32_di_open(STM32_DI_BTM_STATUS,opt);
    break;
    case DI_RAIN_DETECT:
    driver = stm32_di_open(STM32_DI_RAIN_DETECT,opt);
    break;

  }

return driver;

}

void driver_di_close(driver_t *drv)
{
 const di_api_t *api =drv->api;

  api->close(drv);
}

int32_t driver_di_read(driver_t *drv)
{
  const di_api_t *api =drv->api;
  
  return api->read(drv);

}

void driver_di_set(driver_t *drv,uint8_t cmd,void *option)
{
 const di_api_t *api =drv->api;

  api->set(drv,cmd,option); 

}