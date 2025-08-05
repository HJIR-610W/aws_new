#include "app_screen.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "driver_lcd.h"
#include "driver_lcd.h"
#include "cmsis_os2.h"
#include "cli_key_code.h"

#define MAX_COLS 21




static driver_t *p_s_lcd = NULL;
static screen_instance_t s_screen;

void screen_init(void)
{
  p_s_lcd = driver_lcd_open(DRIVER_LCD_TERMNINAL);
  if (p_s_lcd)
  {
    driver_lcd_display_on(p_s_lcd);
    s_screen.height_pixcel = 64;
    s_screen.width_pixel = 128;
    s_screen.font_rows = 8;
    s_screen.font_cols = 21;
  }
}
screen_instance_t* screen_get_instance(void)
{
  return &s_screen;
}

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


void screen_clear(void)
{
  for (int row = 0; row <s_screen.font_rows; row++)
  {
    for (int i = 0; i < s_screen.font_cols; i++)
    {
      screen_put_ch(row, i, ' ');
    }
  }


}

void screen_page_create(screen_page_t* win, int rows, int cols)
{
  win->current_row = 0;
  win->view_row = s_screen.font_rows;

  win->view_col = s_screen.font_cols;

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

void screen_menu_create(screen_menu_t* win,const char *titile)
{
  win->current_row = 0;
  win->view_row = s_screen.font_rows;
  win->view_col = s_screen.font_cols;

  if (win->view_col > MAX_COLS)
  {
    win->view_col = MAX_COLS;
  }

  win->scroll_offset = 0;
  win->total_items = 0;
  win->selected_index = 0;  // 첫 번째 메뉴 항목이 기본 선택
  
  if(titile)
  {
    snprintf(win->title,sizeof(win->title),"%s",titile);
  }
  else
  {
    memset(win->title, 0, sizeof(win->title));  // 타이틀 초기화
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
  for (i = col +len; i < MAX_COLS; i++)
  {
    screen_put_ch(row, i, ' ');
  }
}

void screen_printf_row(screen_page_t* win, int row_index, const char* format, ...)
{
  char s_format_buffer[MAX_COLS+1];  // 정적 버퍼 크기는 필요에 따라 조정
  va_list args;
  int display_row;
  int cols;
  int i;
  int text_len;
  int page;

  win->total_items[win->current_page]++;


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

void screen_page_handle(screen_page_t* win, int key)
{
  int page = win->current_page;
  int new_offset = 0;

  switch (key)
  {
    case KEY_CODE_UP:  // 위로 스크롤
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
    case KEY_CODE_DOWN:  // 아래로 스크롤
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
    case KEY_CODE_LEFT:  // 이전 페이지
      if (win->multi_page_use && (win->current_page > 0))
      {
        win->current_page--;
      }
      break;
    case KEY_CODE_RIGHT:  // 다음 페이지
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


    void screen_menu_printf_row(screen_menu_t *win, int row_index, const char *format, ...)
{
  char s_format_buffer[MAX_COLS];  // 정적 버퍼 크기는 필요에 따라 조정
  va_list args;
  int display_row;
  int cols;
  int i;
  int text_len;
  char selection_indicator;
  int title_offset = 0;

  win->total_items++;

  // 타이틀이 있으면 항상 첫 번째 행에 타이틀 표시 
  if (strlen(win->title) > 0)
  {
    if (win->current_row == 0)
    {
      screen_set_cursor(0, 0);
      text_len = strlen(win->title);
      
      // 타이틀은 선택 표시 없이 출력
      for (i = 0; i < text_len && i < win->view_col; i++)
      {
        screen_put_ch(0, i, win->title[i]);
      }
      
      // 나머지 공간을 공백으로 채움
      for (i = text_len; i < win->view_col; i++)
      {
        screen_put_ch(0, i, ' ');
      }
      
      win->current_row++;
    }
    title_offset = 1;
  }

  if (win->current_row >= win->view_row)
  {
    return;
  }

  // 가변 인자를 문자열로 포맷팅
  va_start(args, format);
  vsnprintf(s_format_buffer, sizeof(s_format_buffer), format, args);
  va_end(args);

  cols = win->view_col;

  // 타이틀 오프셋을 고려하여 스크롤 범위 조정
  if (row_index >= win->scroll_offset && row_index < win->scroll_offset + win->view_row - title_offset)
  {
    display_row = row_index - win->scroll_offset + title_offset;

    screen_set_cursor(display_row, 0);

    text_len = strlen(s_format_buffer);

    // 선택된 항목이면 '*', 아니면 ' ' 표시
    selection_indicator = (win->selected_index == row_index) ? '*' : ' ';
    screen_put_ch(display_row, 0, selection_indicator);

    // 텍스트 출력 (첫 번째 문자부터 시작)
    for (i = 0; i < text_len && (i + 1) < cols; i++)
    {
      screen_put_ch(display_row, i + 1, s_format_buffer[i]);
    }

    // 나머지 공간을 공백으로 채움
    for (i = text_len + 1; i < cols; i++)
    {
      screen_put_ch(display_row, i, ' ');
    }

    win->current_row++;
  }
  
  // total_items 자동 업데이트
  if (row_index >= win->total_items)
  {
    win->total_items = row_index + 1;
  }
}
void screen_menu_handle(screen_menu_t* win, int key)
{
  int title_offset = (strlen(win->title) > 0) ? 1 : 0;// 타이틀이 존재 하면 
  int effective_view_row = win->view_row - title_offset;// 타이틀을 제외한 행만 유효한 표시행 
  
  switch (key)
  {
    case KEY_CODE_UP:  // 위로 이동
      if (win->selected_index > 0)
      {
        win->selected_index--;
    
        // 선택된 항목이 화면 위쪽을 벗어나면 스크롤
        if (win->selected_index < win->scroll_offset)
        {
          win->scroll_offset = win->selected_index;
        }
      }
      break;

    case KEY_CODE_DOWN:  // 아래로 이동
      if (win->selected_index < win->total_items - 1)
      {
        win->selected_index++;
        
        // 선택된 항목이 화면 아래쪽을 벗어나면 스크롤
        if (win->selected_index >= win->scroll_offset + effective_view_row)
        {
          win->scroll_offset = win->selected_index - effective_view_row + 1;
        }
      }
      break;
  }
}

void screen_menu_clear_row(screen_menu_t* win, int row_index)
{
  int i;
  int display_row;
  int title_offset = (strlen(win->title) > 0) ? 1 : 0;


  display_row = row_index - win->scroll_offset + title_offset;

  // 나머지 공간을 공백으로 채움
  for (i = 0; i < win->view_col; i++)
  {
    screen_put_ch(display_row, i, ' ');
  }
  
  win->current_row++;
}

void screen_menu_start(screen_menu_t *win)
{
  win->current_row = 0;
  win->total_items = 0;
}
void screen_menu_clear(screen_menu_t *win)
{
  int i;
  int display_row;


  display_row = win->current_row - win->scroll_offset ;

  for (; display_row < win->view_row; display_row++)
  {
    // 나머지 공간을 공백으로 채움
    for (i = 0; i < win->view_col; i++)
    {
      screen_put_ch(display_row, i, ' ');
    }
  }
}

void screen_off(void)
{
  screen_clear();
  screen_printf(3, 0, "      Screen Off");
  screen_refresh();
}

void screen_on(void)
{
  screen_clear();
  screen_printf(3, 0, "      Screen On");
  screen_refresh();
  osDelay(1000);
}


void screen_update_list(screen_menu_t * p_screen,int index,int id)
{
  if(index<sizeof(p_screen->index_list))
  p_screen->index_list[index] = id;
}

void screen_clear_unsued_line(screen_menu_t* p_win)
{
  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, p_win->current_row );
  }
}

/**
 * @brief win->currrent_row기준으로 view_row 남은 행을 전부 공백표시,clear
 */
void screen_page_clear(screen_page_t *win)
{
  int i;
  int display_row;
  int page = win->current_page;


  display_row = win->current_row - win->scroll_offset[page];

  for (; display_row <  win->view_row; display_row++)
  {
    // 나머지 공간을 공백으로 채움
    for (i = 0; i < win->view_col; i++)
    {
      screen_put_ch(display_row, i, ' ');
    }
  }

}

void screen_page_start(screen_page_t *win)
{
  int page = win->current_page;
  win->current_row  = 0;
  win->total_items[page] = page;
}