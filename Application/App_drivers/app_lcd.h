#ifndef APP_LCD_H
#define APP_LCD_H

#include <stdint.h>
#include <stdarg.h>

void clcd_init(void);
void clcd_printf(int row, int col, char const* const _Format, ...);
void clcd_clear(void);
void clcd_home(void);
void clcd_display_on(void);
void clcd_display_off(void);
void clcd_set_position(int row, int col);
void clcd_write_string(const char *str);

#endif