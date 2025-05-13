
#ifndef APP_BSP_H
#define APP_BSP_H

void app_bsp_init(void);
float read_battery(void);
float read_temperature(void);

#define LED_BLINK 0
#define LED_ON 1

void status_led_on(void);
void status_led_off(void);
void status_led_set(int mode);

void cdma_power_on(void);
void cdma_power_off(void);
void set_portd_hart_mode(void);
void set_portd_rs232_mode(void);
#endif