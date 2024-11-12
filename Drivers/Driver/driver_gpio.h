

#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H

#include <stdint.h>

#include "driver_interface.h"
#include "driver_gpio_def.h"

#define DRIVER_PCF8575 0


#define DRIVER_PCF8575 0


driver_t *driver_gpio_open(uint32_t num);
int driver_gpio_write(driver_t *drv,uint16_t port_data);
int driver_gpio_read(driver_t *drv,uint16_t *port_data);
int driver_gpio_write_pin(driver_t *drv,uint16_t pin, uint16_t set);
uint16_t driver_gpio_read_pin(driver_t *drv,uint16_t pin);
#endif
