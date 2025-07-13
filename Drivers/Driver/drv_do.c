#include "bsp_do.h"
#include "drv_do.h"

void drv_do_init(void)
{
  bsp_do_init();
}


void drv_do_low(int num)
{
  switch (num)
  {
    case DRV_DO_EXT_0:  
      bsp_do_low(BSP_DO_EXT_0);
       break;
    case DRV_DO_EXT_1:  
      bsp_do_low(BSP_DO_EXT_1);
      break;
    case DRV_DO_EXT_2: 
      bsp_do_low(BSP_DO_EXT_2);
      break;
    case DRV_DO_EXT_3: 
      bsp_do_low(BSP_DO_EXT_3);
      break;
    case DRV_DO_EXT_4: 
      bsp_do_low(BSP_DO_EXT_4);
      break;
    case DRV_DO_EXT_5: 
      bsp_do_low(BSP_DO_EXT_5);
      break;
    default:
      break;
  }

}

void drv_do_high(int num)
{
  switch (num)
  {
    case DRV_DO_EXT_0:
      bsp_do_high(BSP_DO_EXT_0);
      break;
    case DRV_DO_EXT_1:
      bsp_do_high(BSP_DO_EXT_1);
      break;
    case DRV_DO_EXT_2:
      bsp_do_high(BSP_DO_EXT_2);
      break;
    case DRV_DO_EXT_3:
      bsp_do_high(BSP_DO_EXT_3);
      break;
    case DRV_DO_EXT_4:
      bsp_do_high(BSP_DO_EXT_4);
      break;
    case DRV_DO_EXT_5:
      bsp_do_high(BSP_DO_EXT_5);
      break;
    default:
      break;
  }
}