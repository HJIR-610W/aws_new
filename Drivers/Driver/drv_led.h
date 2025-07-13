
#ifndef DRV_LED_H
#define DRV_LED_H

#define DRV_LED_RUN 0

void drv_led_init(void);
void drv_led_on(int led_number);
void drv_led_off(int led_number);
void drv_led_toggle(int led_number);
void drv_led_deinit(int led_number);

#endif