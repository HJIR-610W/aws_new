#include "bsp_do.h"
#include "bsp_power.h"


void bsp_power_init(void)
{
  //bsp do에서 초기화됨
}

void bsp_power_on(int num)
{
  switch(num)
  {
    case BSP_POWER_CDMA:
      bsp_do_high(BSP_DO_POWER_CDMA);
    break;
    case BSP_POWER_HART_24V:
      bsp_do_high(BSP_DO_POWER_HART_24V);
      break;
    case BSP_POWER_LCD:
      bsp_do_high(BSP_DO_POWER_LCD);
      break;
    case BSP_POWER_RAIN_DECT_ANALOG:
      bsp_do_high(BSP_DO_POWER_RAIN_DECT_ANALOG);
      break;
    case BSP_POWER_RAIN_DECT_DIGITAL:
      bsp_do_high(BSP_DO_POWER_RAIN_DECT_DIGITAL);
      break;
  }

}

void bsp_power_off(int num)
{
  switch (num)
  {
    case BSP_POWER_CDMA:
      bsp_do_low(BSP_DO_POWER_CDMA);
      break;
    case BSP_POWER_HART_24V:
      bsp_do_low(BSP_DO_POWER_HART_24V);
      break;
    case BSP_POWER_LCD:
      bsp_do_low(BSP_DO_POWER_LCD);
      break;
    case BSP_POWER_RAIN_DECT_ANALOG:
      bsp_do_low(BSP_DO_POWER_RAIN_DECT_ANALOG);
      break;
    case BSP_POWER_RAIN_DECT_DIGITAL:
      bsp_do_low(BSP_DO_POWER_RAIN_DECT_DIGITAL);
      break;
  }
}
