
#include "driver_digitalOut.h"
#include "driver_stm32_do.h"
#include "main.h"
#include "stm32f4xx_hal.h"


typedef struct do_api_s
{
  void (*low)(driver_t *driver);
  void (*high)(driver_t *driver);
}do_api_t;


static do_api_t do_api={.high = stm32_do_high,
                        .low  = stm32_do_low};


driver_t g_do_list[DO_NUM_MAX];

driver_t *driver_do_open(uint32_t num)
{

  driver_t *p_drv;

  if(g_do_list[num].opened == true)
  {
    return &g_do_list[num];
  }
  switch(num)
  {
    case DO_FRAM_CS:
      p_drv = stm32_do_open(num);
    break;
    case DO_RTC_CS:
      p_drv = stm32_do_open(num);
      break;
    case DO_FLASH_CS:
      p_drv = stm32_do_open(num);
    break;
    case DO_ADC_NCS:
      p_drv = stm32_do_open(num);
    break;
   case DO_CON_PWR_232_A :
         p_drv = stm32_do_open(num);
    break;
   case  DO_CON_PWR_232_B:
         p_drv = stm32_do_open(num);
    break;
   case  DO_CON_PWR_485:
         p_drv = stm32_do_open(num);
    break;
   case DO_CON_PWR_TC:
         p_drv = stm32_do_open(num);
    break;
   case  DO_CON_PWR_DSEN:
         p_drv = stm32_do_open(num);
    break;
   case  DO_CON_PWR_ASEN:
         p_drv = stm32_do_open(num);
    break;
   case  DO_CON_PWR_ASEN_A:
         p_drv = stm32_do_open(num);
    break;
   case  DO_CON_PWR_ASEN_B:
         p_drv = stm32_do_open(num);
    break;
   case  DO_CON_PWR_ASEN_C:
         p_drv = stm32_do_open(num);
    break;
   case  DO_CON_PWR_ASEN_D:
         p_drv = stm32_do_open(num);
    break;
   break;

  }

  g_do_list[num].api = &do_api;
  g_do_list[num].handle = p_drv;//¿¬°áµÈ IC
  g_do_list[num].opened = true; 

    return &g_do_list[num];
}

void driver_do_low(driver_t *drv)
{
  const do_api_t *api =drv->api;

  api->low(drv->handle);
}

void driver_do_high(driver_t *drv)
{
  const do_api_t *api =drv->api;

  api->high(drv->handle);

}