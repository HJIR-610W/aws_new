

#include "driver_digitalIn.h"
#include "driver_stm32_di.h"
#include "main.h"
#include "mcu_interrupt.h"
#include "stm32f4xx_hal.h"


typedef struct di_api_s
{
  int32_t (*read)(driver_t *driver);
  void (*set)(driver_t *drv,uint8_t cmd,void *option);
}di_api_t;


static di_api_t di_api={.read = stm32_di_read,
                        .set  = stm32_di_set};

driver_t g_drv_di_list[DI_MAX];

driver_t *driver_di_open(uint32_t num)
{

  driver_t *p_drv;

  if(g_drv_di_list[num].opened == true)
  {
    return &g_drv_di_list[num];
  }
  switch(num)
  {
    case DI_ADC_RDY:
    case DI_RTC_IRQ:
    case DI_RAIN_HALL:
    case DI_RAIN_REED :
    case DI_RAIN_HALL_ERR:
    case DI_QUAD_UARTA_1:
    case DI_QUAD_UARTB_2:
    case DI_QUAD_UARTC_3:
    case DI_QUAD_UARTD_4:
    case DI_QUAD_UARTA_5:
    case DI_QUAD_UARTB_6:
    case DI_QUAD_UARTC_7:
    case DI_QUAD_UARTD_8:
         p_drv = stm32_di_open(num);
    break;
  }
  g_drv_di_list[num].num = num;
  g_drv_di_list[num].api = &di_api;
  g_drv_di_list[num].handle = p_drv;//¿¬°áµÈ IC
  g_drv_di_list[num].opened = true; 

    return &g_drv_di_list[num];
}


int32_t driver_di_read(driver_t *drv)
{
  const di_api_t *api = (di_api_t *)drv->api;

 return api->read(drv->handle);
}





void driver_di_set(driver_t *drv,uint8_t cmd,void *option)
{
  di_api_t *api = ( di_api_t*)(drv->api);

  api->set(drv->handle,cmd,option);
}