

#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H

#include <stdint.h>

#include "cmsis_os.h"
typedef struct driver_gpio_s
{
    const char *name;//LED ¿Ã∏ß
    uint8_t err;
    osSemaphoreId mutex;
    uint32_t num;
    const void *api;
    void *cfg;
}driver_gpio_t;

#endif