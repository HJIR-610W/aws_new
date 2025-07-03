#ifndef APP_LCD_H
#define APP_LCD_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "driver_lcd.h"
void clcd_init(void);
void clcd_printf(int row, int col, char const* const _Format, ...);
void clcd_clear(void);
void clcd_home(void);
void clcd_display_on(void);
void clcd_display_off(void);
void clcd_set_position(int row, int col);
void clcd_write_string(const char *str);
void clcd_write_string_at(int row, int col, const char *str);

void clcd_set_mode(eLCD_MODE_t lcd_mode);
void clcd_set_pixel( uint8_t x, uint8_t y, bool on);
void clcd_flush_buffer(void);
#endif