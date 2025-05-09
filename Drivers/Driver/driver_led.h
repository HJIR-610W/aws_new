

#ifndef DRIVER_LED_H
#define DRIVER_LED_H


#include <stdint.h>

#include "driver_interface.h"
#include "cmsis_os.h"




#define LED_SYS_RUN   0


#define LED_CMD_SET_TOGGLE_FREQ 1
#define LED_CMD_START           2
#define LED_CMD_STOP            3

typedef struct led_freq_cfg_s
{
  uint32_t freq;
  uint16_t highDuty;
}led_freq_cfg_t;

driver_t * driver_led_open(uint32_t num);
void driver_led_on(driver_t *led);
void driver_led_off(driver_t *led);
void driver_led_toggle(driver_t *led);
void driver_led_set(driver_t *drv,uint8_t cmd,void *option);

#endif
