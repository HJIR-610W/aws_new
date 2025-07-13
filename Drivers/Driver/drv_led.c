
#include "bsp_led.h"


void drv_led_init(void)
{
  bsp_led_init();
}

void drv_led_on(int led_number)
{
  bsp_led_on(led_number);
}

void drv_led_off(int led_number)
{
  bsp_led_off(led_number);
}

void drv_led_toggle(int led_number)
{
  bsp_led_toggle(led_number);
}

void drv_led_deinit(int led_number)
{
  bsp_led_deinit(led_number);
}