#include "app_lcd.h"
#include "driver_lcd.h"
#include <stdio.h>
#include <stdarg.h>
#include "lcd/font_6x8.h"
#include "lcd/hangul_font.h"

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

void clcd_refresh(void)
{
  st7920_flush_buffer(p_s_lcd);
}

void clcd_put_ch(int row, int col, uint8_t ch)
{
    if(p_s_lcd == NULL) return;
    
    // Check if character is in printable range
    if(ch < 0x20 || ch > 0x7E) return;
    
    // Calculate position in pixels (6x8 font)
    int start_x = 1+col * FONT_6X8_WIDTH;
    int start_y = row * FONT_6X8_HEIGHT;
    
    // Check bounds for 128x64 display
    if(start_x + FONT_6X8_WIDTH > 128 || start_y + FONT_6X8_HEIGHT > 64) return;
    
    // Get character data from font table (ch - 0x20 gives index)
    const uint8_t *char_data = font_6x8[ch - 0x20];
    
    // Draw character pixel by pixel
    for (int y = 0; y < FONT_6X8_HEIGHT; y++)
    {
        uint8_t row_data = char_data[y];
        for (int x = 0; x < FONT_6X8_WIDTH; x++)
        {
            if (row_data & (0x10 >> x))  // Check bit from bit 4 (6-bit font uses bits 4-0)
            {
                clcd_set_pixel(start_x + x, start_y + y, 1);
            }
            else
            {
                              clcd_set_pixel(start_x + x, start_y + y, 0);
            }
        }
    }
}

void clcd_put_hangul(int row, int col, uint16_t unicode)
{
    if(p_s_lcd == NULL) return;
    
    // Find the character in hangul_chars array
    for(int i = 0; i < hangul_chars_count; i++)
    {
        if(hangul_chars[i].unicode == unicode)
        {
            // Calculate position in pixels (16x16 font)
            int start_x = col * HANGUL_FONT_WIDTH;
            int start_y = row * HANGUL_FONT_HEIGHT;
            
            // Check bounds for 128x64 display
            if(start_x + HANGUL_FONT_WIDTH > 128 || start_y + HANGUL_FONT_HEIGHT > 64) return;
            
            // Draw character pixel by pixel
            const uint8_t *char_data = hangul_chars[i].bitmap;
            for (int y = 0; y < HANGUL_FONT_HEIGHT; y++)
            {
                uint16_t row_data = (char_data[y*2] << 8) | char_data[y*2 + 1];
                for (int x = 0; x < HANGUL_FONT_WIDTH; x++)
                {
                    if (row_data & (0x8000 >> x))  // Check bit from MSB
                    {
                        clcd_set_pixel(start_x + x, start_y + y, 1);
                    }
                }
            }
            return;
        }
    }
}


