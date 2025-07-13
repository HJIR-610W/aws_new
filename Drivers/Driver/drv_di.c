
#include "bsp.h"

#include "drv_di.h"
#include "bsp_di.h"


void drv_di_init(void)
{
  bsp_di_init();
}


int32_t drv_di_read(int di_number)
{
  switch (di_number)
  {
    case DRV_DI_0:
      return bsp_di_read(BSP_DI_0);
    case DRV_DI_1:
      return bsp_di_read(BSP_DI_1);
    case DRV_DI_2:
      return bsp_di_read(BSP_DI_2);
    case DRV_DI_3:
      return bsp_di_read(BSP_DI_3);
    case DRV_DI_4:
      return bsp_di_read(BSP_DI_4);
    case DRV_DI_5:
      return bsp_di_read(BSP_DI_5);
    case DRV_DI_6:
      return bsp_di_read(BSP_DI_6);
    case DRV_DI_7:
      return bsp_di_read(BSP_DI_7);
    case DI_RAIN_REED:
      return bsp_di_read(BSP_DI_RAIN_REED);
    case DI_RAIN_HALL:
      return bsp_di_read(BSP_DI_RAIN_HALL);
    case DI_RAIN_HALL_ERR:
      return bsp_di_read(BSP_DI_RAIN_HALL_ERR);
    case DI_USER_BTN:
      return bsp_di_read(BSP_DI_USER_BTN);
    case DI_RAIN_DETECT:
      return bsp_di_read(BSP_DI_RAIN_DETECT);
    default:
    {
      return -1;
    }
  }

  return -1;
}

void  drv_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg)
{

}