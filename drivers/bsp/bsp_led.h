

#ifndef BSP_LED_H
#define BSP_LED_H


#include <stdint.h>
#include "driver_interface.h"

#define BSP_LED_RUN 0
#define BSP_LED_MAX 1

void bsp_led_init(void);
void bsp_led_on(int led_number);
void bsp_led_off(int led_number);
void bsp_led_toggle(int led_number);
void bsp_led_deinit(int led_number);

#endif
