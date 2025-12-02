#include "drv_rtc.h"
#include "drv_led.h"
#include "drv_di.h"
#include "drv_do.h"
#include "drv_power.h"
#include "drv_system.h"
#include "drv_fram.h"
#include "drv_flash.h"
#include "drv_adc.h"
#include "drv_rs485.h"
#include "drv_rs232.h"
#include "drv_frequency.h"
#include "drv_crc.h"

void drv_init(void)
{
  drv_do_init();
  drv_power_init();
  drv_di_init();
  drv_crc_init();
  drv_system_init();
  drv_flash_init(); 
  drv_fram_init();
  drv_adc_init();
  drv_frequency_init();
  drv_led_init();
  drv_led_on(DRV_LED_RUN);
  drv_rtc_init();
  drv_rtc_read(&Date_Time);
}