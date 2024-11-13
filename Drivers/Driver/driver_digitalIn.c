

#include "driver_digitalIn.h"
#include "driver_stm32_di.h"
#include "main.h"
#include "mcu_interrupt.h"
#include "stm32f4xx_hal.h"


typedef struct di_api_s
{
  int32_t (*read)(driver_t *driver);
}di_api_t;


static di_api_t di_api={.read = stm32_di_read};

driver_t g_di_list[DI_MAX];

driver_t *driver_di_open(uint32_t num)
{

  driver_t *p_drv;

  if(g_di_list[num].opened == true)
  {
    return &g_di_list[num];
  }
  switch(num)
  {
    case DI_ADC_RDY:
      p_drv = stm32_di_open(num);
    break;
    case DI_RTC_IRQ:
      p_drv = stm32_di_open(num);
      break;

  }

  g_di_list[num].api = &di_api;
  g_di_list[num].handle = p_drv;//¿¬°áµÈ IC
  g_di_list[num].opened = true; 

    return &g_di_list[num];
}


int32_t driver_di_read(driver_t *drv)
{
  const di_api_t *api = (di_api_t *)drv->api;

 return api->read(drv->handle);
}
