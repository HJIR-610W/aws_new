
#ifndef APP_BSP_H
#define APP_BSP_H

#include <stdint.h>
#include <stdbool.h>

#define LED_BLINK 0
#define LED_ON 1


void app_bsp_init(void);

void status_led_on(void); //MCU RUN LED
void status_led_off(void);
void status_led_set(int mode);
bool door_opened(void);  // 도어 상태 감지


void set_portd_hart_mode(void); //rs232_d 포트 사용 모드 HART/RS232
void set_portd_rs232_mode(void);


#endif