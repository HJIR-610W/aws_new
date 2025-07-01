
#ifndef DRIVER_LCD_DEFINE_H
#define DRIVER_LCD_DEFINE_H

#include "driver_interface.h"
typedef struct gpio_api_s
{
  void (*set_position)(driver_t *drv, uint8_t x, uint8_t y);
  void (*write_string)(driver_t *drv, const char *str);
  void (*clear_screen)(driver_t *drv);
  void (*home)(driver_t *drv);
  void (*display_on)(driver_t *drv);
  void (*display_off)(driver_t *drv);
} lcd_api_t;
#endif