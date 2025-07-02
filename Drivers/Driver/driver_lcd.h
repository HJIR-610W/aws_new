




#ifndef DRIVER_LCD_H
#define DRIVER_LCD_H

#include "driver_interface.h"

#define DRIVER_CLCD 0
#define DRIVER_LCD_TERMNINAL 1

driver_t *driver_lcd_open(int num);

void driver_lcd_set_position(driver_t *drv, uint8_t x, uint8_t y);
void driver_lcd_write_string(driver_t *drv, const char *str);
void driver_lcd_clear_screen(driver_t *drv);
void driver_lcd_home(driver_t *drv);
void driver_lcd_display_on(driver_t *drv);
void driver_lcd_display_off(driver_t *drv);
#endif