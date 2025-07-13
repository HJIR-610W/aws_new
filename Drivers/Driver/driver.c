

#include "drv_rtc.h"
#include "drv_led.h"
#include "drv_di.h"
#include "drv_do.h"
#include "drv_power.h"
void drv_init(void)
{
  drv_rtc_init();
  drv_led_init();
  drv_do_init();
  drv_di_init();
  drv_power_init();
  
}