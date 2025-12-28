#include "app_screen.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "drv_lcd.h"
#include "drv_lcd.h"
#include "cmsis_os2.h"
#include "cli_key_code.h"
#include "util_memory.h"
#include "drv_power.h"
#include "debug_io.h"

#define MAX_COLS 21
#define MAX_ROWS 8


static driver_t *p_s_lcd = NULL;
static screen_instance_t s_screen;


uint8_t g_debug_port_mirror_enable = 0;
void screen_init(void)
{
#ifdef NOT_USE_LCD
    p_s_lcd = driver_lcd_open(DRIVER_LCD_TERMNINAL);
#else
    p_s_lcd = driver_lcd_open(DRIVER_CLCD);
#endif

  if (p_s_lcd)
  {
    driver_lcd_display_on(p_s_lcd);

    s_screen.height_pixcel = 64;
    s_screen.width_pixel = 128;
    s_screen.font_rows = MAX_ROWS;
    s_screen.font_cols = MAX_COLS;
    s_screen.screen_on  = true;
    s_screen.graphic_mode = true;
    
  }
}
screen_instance_t* screen_get_instance(void)
{
  return &s_screen;
}








void screen_set_pixel( uint8_t x, uint8_t y, bool on)
{
    driver_lcd_set_pixel(p_s_lcd,  x,  y,  on);
}


void screen_refresh(void)
{
  driver_lcd_flush(p_s_lcd);
}

void screen_put_ch(int row, int col, uint8_t ch)
{
  driver_lcd_put_ch(p_s_lcd,row,col,ch);
  if(g_debug_port_mirror_enable == 1)
  {
    VT100_PUT_CH_AT(row+1,col+1,ch);
  }
}

void screen_clear(void)
{
  driver_lcd_clear_screen(p_s_lcd);
  if(g_debug_port_mirror_enable == 1)
  {
    VT100_CLEAR();
  }
}



void screen_printf(int row, int col, const char* format, ...)
{
  char s_format_buffer[MAX_COLS+1];  // 정적 버퍼 크기는 필요에 따라 조정
  int i;
  int len;
  va_list args;

  va_start(args, format);
  vsnprintf(s_format_buffer, sizeof(s_format_buffer), format, args);
  va_end(args);


  len = strlen(s_format_buffer);

  for (i = 0; i < len && i < MAX_COLS; i++)
  {
    screen_put_ch(row, col+i, s_format_buffer[i]);
  }

  // 나머지 공간을 공백으로 채움
  for (i = col +len; i < MAX_COLS; i++)
  {
    screen_put_ch(row, i, ' ');
  }
}

void screen_puts(int row,int col,const char *string)
{
  int i;
  int len;
  
   len = strlen(string);

  for (i = 0; i < len && i < MAX_COLS; i++)
  {
    screen_put_ch(row, col+i, string[i]);
  }

  for (i = col +len; i < MAX_COLS; i++)
  {
    screen_put_ch(row, i, ' ');
  }
}


void screen_off(void)
{
  screen_printf(3, 0, "      Screen Off");
  osDelay(1000);
  screen_clear();
  screen_refresh();
  drv_lcd_close(DRIVER_CLCD);
  s_screen.screen_on = false;
}

void screen_on(void)
{
  screen_init();
  //screen_clear();
  screen_printf(3, 0, "      Screen On");
  screen_refresh();
  osDelay(1000);

}



