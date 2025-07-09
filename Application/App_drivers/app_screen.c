#include "app_screen.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "driver_lcd.h"
#include "cmsis_os2.h"

#define MAX_COLS 21


static driver_t *p_s_lcd = NULL;


void screen_home(void)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_home(p_s_lcd);
}

void screen_display_on(void)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_display_on(p_s_lcd);
}

void screen_display_off(void)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_display_off(p_s_lcd);
}

void screen_set_cursor(int row, int col)
{
    if(p_s_lcd == NULL) return;
    driver_lcd_set_position(p_s_lcd, row, col);
}

void screen_set_mode(eLCD_MODE_t lcd_mode)
{
   driver_lcd_set_mode(p_s_lcd,  lcd_mode);
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
}


void screen_clear(screen_page_t* p_win)
{
  for (int row = 0; row < p_win->view_row; row++)
  {
    for (int i = 0; i < p_win->view_col; i++)
    {
      screen_put_ch(row, i, ' ');
    }
  }

  p_win->current_row = 0;
}

void screen_page_create(screen_page_t* win, int rows, int cols)
{
  win->current_row = 0;
  win->view_row = rows;

  win->view_col = cols;

  if (cols > MAX_COLS)
  {
    win->view_col = MAX_COLS;
  }

  win->current_page = 0;
  win->total_pages = 1;

  for (int i = 0; i < SCREEN_PAGE_MAX; i++)
  {
    win->scroll_offset[i] = 0;
    win->total_items[i] = 0;
  }
}

void screen_printf(int row, int col, const char* format, ...)
{
  char s_format_buffer[MAX_COLS];  // 정적 버퍼 크기는 필요에 따라 조정
  va_list args;
  int i;
  int len;
  va_start(args, format);
  vsnprintf(s_format_buffer, sizeof(s_format_buffer), format, args);
  va_end(args);

  screen_set_cursor(row, col);

  len = strlen(s_format_buffer);

  // 텍스트 출력
  for (i = 0; i < len && i < MAX_COLS; i++)
  {
    screen_put_ch(row, col+i, s_format_buffer[i]);
  }

  // 나머지 공간을 공백으로 채움
  for (i = len; i < MAX_COLS; i++)
  {
    screen_put_ch(row, i, ' ');
  }
}

void screen_printf_row(screen_page_t* win, int row_index, const char* format, ...)
{
  char s_format_buffer[MAX_COLS];  // 정적 버퍼 크기는 필요에 따라 조정
  va_list args;
  int display_row;
  int cols;
  int i;
  int text_len;
  int page;

  if (win->current_row >= win->view_row)
  {
    return;
  }

  // 가변 인자를 문자열로 포맷팅
  va_start(args, format);
  vsnprintf(s_format_buffer, sizeof(s_format_buffer), format, args);
  va_end(args);

  cols = win->view_col;
  page = win->current_page;

  if (row_index >= win->scroll_offset[page] && row_index < win->scroll_offset[page] + win->view_row)
  {
    display_row = row_index - win->scroll_offset[page];

    screen_set_cursor(display_row, 0);

    text_len = strlen(s_format_buffer);

    // 텍스트 출력
    for (i = 0; i < text_len && i < cols; i++)
    {
      screen_put_ch(display_row, i, s_format_buffer[i]);
    }

    // 나머지 공간을 공백으로 채움
    for (i = text_len; i < cols; i++)
    {
      screen_put_ch(display_row, i, ' ');
    }

    win->current_row++;
  }
}

void screen_clear_row(screen_page_t* win, int row_index)
{
  int i;
  int display_row;
  int page = win->current_page;

  if (win->current_row >= win->view_row)
  {
    return;
  }

  display_row = row_index - win->scroll_offset[page];

  // 나머지 공간을 공백으로 채움
  for (i = 0; i < win->view_col; i++)
  {
    screen_put_ch(display_row, i, ' ');
  }
  win->current_row++;
}

void screen_handle_scroll(screen_page_t* win, int key)
{
  int page = win->current_page;
  int new_offset = 0;

  switch (key)
  {
    case '8':  // 위로 스크롤
      if (win->scroll_offset[page] > 0)
      {
        if(win->chunk_scroll_use)
        {
        win->scroll_offset[page] -= win->view_row;
        }
        else{
          win->scroll_offset[page] -= 1;
        }
        if (win->scroll_offset[page] < 0)
        {
          win->scroll_offset[page] = 0;
        }
      }
      break;
    case '2':  // 아래로 스크롤
      if(win->chunk_scroll_use)
      {
        new_offset = win->scroll_offset[page] + win->view_row;
      }
      else
      {
        new_offset = win->scroll_offset[page] + 1;
      }


      if (new_offset < win->total_items[page])
      {
        win->scroll_offset[page] = new_offset;
      }
      break;
    case '4':  // 이전 페이지
        if (win->multi_page_use&&(win->current_page > 0))
        {
          win->current_page--;
        }
      break;
    case '6':  // 다음 페이지
      if (win->multi_page_use)
      {
        if (win->current_page < win->total_pages - 1)
        {
          win->current_page++;
        }
      }
      break;
  }
}

void screen_off(screen_page_t* p_screen)
{
  screen_clear(p_screen);
  screen_printf_row(p_screen, 3, "      Screen Off");
  screen_refresh();


}

void screen_on(screen_page_t* p_screen)
{

  screen_clear(p_screen);
  screen_printf_row(p_screen, 3, "      Screen On");
  screen_refresh();
  osDelay(1000);
}

void screen_init(void)
{
  p_s_lcd = driver_lcd_open(DRIVER_CLCD);
  if (p_s_lcd)
  {
    driver_lcd_display_on(p_s_lcd);
  }
}
