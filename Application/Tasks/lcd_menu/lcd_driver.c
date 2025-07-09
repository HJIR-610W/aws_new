#include "lcd_driver.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#include "app_lcd.h"
#include "cli_key_code.h"
#include "cmsis_os2.h"
#define MAX_COLS 100


void screen_init(void)
{
  clcd_init();
}

void screen_set_cursor(int row, int col)
{
  clcd_set_position(row,col);
}


void screen_clear(screen_t *p_win)
{
  for(int row = 0 ; row <p_win->view_row;row++)
  {
    for (int i = 0; i < p_win->view_col; i++)
    {
      clcd_put_ch(row, i, ' ');
    }
  }

  p_win->current_row = 0;

}

void screen_create(screen_t* win, int rows, int cols)
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

void screen_print_row(screen_t* win, int row_index, const char* text)
{
  int display_row;
  int cols;
  int i;
  int text_len;
  int page;
  
  cols = win->view_col;

  if (win->current_row >= win->view_row)
    return;
  
  page = win->current_page;
  

  if (row_index >= win->scroll_offset[page] && 
    row_index < win->scroll_offset[page] + win->view_row)
    {
       display_row = row_index - win->scroll_offset[page];
    
      screen_set_cursor(display_row, 0);

      text_len = strlen(text);

      for ( i = 0; i < text_len; i++)
      {
        clcd_put_ch(display_row, i, text[i]);
      }

      for ( i = text_len; i < cols; i++)
      {
        clcd_put_ch(display_row, i,' ');
      }

    win->current_row++;
  }
}



void screen_printf_row(screen_t* win, int row_index, const char* format, ...)
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
      clcd_put_ch(display_row, i, s_format_buffer[i]);
    }

    // 나머지 공간을 공백으로 채움
    for (i = text_len; i < cols; i++)
    {
      clcd_put_ch(display_row, i, ' ');
    }

    win->current_row++;
  }
}



void screen_clear_row(screen_t* win, int row_index)
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
    clcd_put_ch(display_row, i, ' ');
  }
    win->current_row++;
}

  void screen_handle_scroll(screen_t * win, int key)
  {
    int page = win->current_page;
    int new_offset = 0;

    switch (key)
    {
      case '8':  // 위로 스크롤
        if (win->scroll_offset[page] > 0)
        {
          win->scroll_offset[page] -= win->view_row;
          if (win->scroll_offset[page] < 0)
          {
            win->scroll_offset[page] = 0;
          }
        }
        break;
      case '2':  // 아래로 스크롤
        new_offset = win->scroll_offset[page] + win->view_row;

        if (new_offset < win->total_items[page])
        {
          win->scroll_offset[page] = new_offset;
        }
        break;
      case '4':  // 이전 페이지
       if (win->current_page > 0)
       {
          win->current_page--;
       }
       break;
       case '6':  // 다음 페이지
       if (win->current_page < win->total_pages - 1)
       {
          win->current_page++;
       }
       break;
  }
}



void screen_off(screen_t *p_screen)
{
  screen_clear(p_screen);
  screen_printf_row(p_screen, 3, "      Screen Off");
  clcd_refresh();
}

void screen_on(screen_t* p_screen)
{
  screen_clear(p_screen);
  screen_printf_row(p_screen, 3, "      Screen On");
  clcd_refresh();
  osDelay(1000);
}