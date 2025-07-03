#include "app_lcd.h"
#include "driver_lcd.h"
#include <stdio.h>
#include <stdarg.h>

static driver_t *p_s_lcd = NULL;

void clcd_init(void)
{
    p_s_lcd = driver_lcd_open(DRIVER_CLCD);
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
    
    driver_lcd_set_position(p_s_lcd, row, col);
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
    driver_lcd_set_position(p_s_lcd, row, col);
}

void clcd_write_string(const char *str)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_write_string(p_s_lcd, str);
}

void clcd_write_string_at(int row, int col, const char *str)
{
    if(p_s_lcd == NULL) return;
    
    // 새로운 driver_lcd_write_string_at 함수 사용
    driver_lcd_write_string_at(p_s_lcd, row, col, str);
}



void clcd_set_mode(eLCD_MODE_t lcd_mode)
{
   driver_lcd_set_mode(p_s_lcd,  lcd_mode);

}
void clcd_set_pixel( uint8_t x, uint8_t y, bool on)
{

 driver_lcd_set_pixel(p_s_lcd,  x,  y,  on);
}

extern void st7920_flush_buffer(driver_t *drv);

void clcd_flush_buffer(void)
{
  st7920_flush_buffer(p_s_lcd);
}


