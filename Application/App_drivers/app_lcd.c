#include "app_lcd.h"
#include "driver_lcd.h"
#include <stdio.h>
#include <stdarg.h>

static driver_t *p_s_lcd = NULL;

void clcd_init(void)
{
    p_s_lcd = driver_lcd_open(DRIVER_LCD);
    if(p_s_lcd)
    {
        driver_lcd_display_on(p_s_lcd);
    }
}

void clcd_printf(int row, int col, char const* const _Format, ...)
{
    if(p_s_lcd == NULL) return;
    
    char buffer[128];
    va_list args;
    
    va_start(args, _Format);
    vsnprintf(buffer, sizeof(buffer), _Format, args);
    va_end(args);
    
    driver_lcd_set_position(p_s_lcd, col, row);
    driver_lcd_write_string(p_s_lcd, buffer);
}

void clcd_clear(void)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_clear_screen(p_s_lcd);
}

void clcd_home(void)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_home(p_s_lcd);
}

void clcd_display_on(void)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_display_on(p_s_lcd);
}

void clcd_display_off(void)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_display_off(p_s_lcd);
}

void clcd_set_position(int row, int col)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_set_position(p_s_lcd, col, row);
}

void clcd_write_string(const char *str)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_write_string(p_s_lcd, str);
}