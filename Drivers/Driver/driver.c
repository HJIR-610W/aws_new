

#include "drv_rtc.h"
#include "drv_led.h"

void drv_init(void)
{
  drv_rtc_init();
  drv_led_init();
}