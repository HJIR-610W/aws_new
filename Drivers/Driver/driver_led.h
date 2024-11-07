

#ifndef DRIVER_LED_H
#define DRIVER_LED_H


#include <stdint.h>

#include "cmsis_os.h"




#define LED_SYS_RUN   0


typedef struct driver_led_s
{
    const char *name;//LED ¿Ã∏ß
    uint8_t err;
    osSemaphoreId mutex;
    uint32_t num;
    void *api;

}driver_led_t;





void driver_led_init(driver_led_t *led,uint32_t num);
void driver_led_on(driver_led_t *led);
void driver_led_off(driver_led_t *led);



#endif