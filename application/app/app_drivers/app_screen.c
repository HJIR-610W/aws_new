#include "app_screen.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "driver_lcd.h"
#include "driver_lcd.h"
#include "cmsis_os2.h"
#include "cli_key_code.h"
#include "util_memory.h"
#include "drv_power.h"

#define MAX_COLS 21
#define MAX_ROWS 8


static driver_t *p_s_lcd = NULL;
static screen_instance_t s_screen;

void screen_init(void)
{
  p_s_lcd = driver_lcd_open(DRIVER_CLCD);
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

void screen_home(void)
{
    if(p_s_lcd == NULL)
    return;
    driver_lcd_home(p_s_lcd);
}


void screen_display_on(void)
{
    if(p_s_lcd == NULL)
     return;
    driver_lcd_display_on(p_s_lcd);
}

void screen_display_off(void)
{
    if(p_s_lcd == NULL)
    return;
    driver_lcd_display_off(p_s_lcd);
}

void screen_set_cursor(int row, int col)
{
    if(p_s_lcd == NULL)
     return;
   // driver_lcd_set_position(p_s_lcd, row, col);
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
  driver_lcd_clear_screen(p_s_lcd);
}

void screen_page_create(screen_page_t* win)
{
  win->p_screen = &s_screen;
  win->current_row = 0;
  win->current_page = 0;
  win->total_pages = 1;

  for (int i = 0; i < SCREEN_PAGE_MAX; i++)
  {
    win->scroll_offset[i] = 0;
    win->total_items[i] = 0;
  }
}

/**
 * @brief 타이틀이 메뉴용
 */
void screen_menu_create(screen_menu_t* win,const char *titile)
{
  win->p_screen = &s_screen;
  win->current_row = 0;
  win->scroll_offset = 0;
  win->total_items = 0;
  win->selected_index = 0;  // 첫 번째 메뉴 항목이 기본 선택
  win->enter_long_key_active = false;
  
  if(titile)
  {
    snprintf(win->title,sizeof(win->title),"%s",titile);//왼쪽 정렬
  }
  else
  {
    memset(win->title, 0, sizeof(win->title));  // 타이틀 초기화
  }
}


/**
 * @brief 프레임 버퍼기만 오직 1행 출력
 */
void screen_printf(int row, int col, const char* format, ...)
{
  char s_format_buffer[MAX_COLS];  // 정적 버퍼 크기는 필요에 따라 조정
  int i;
  int len;
  va_list args;

  va_start(args, format);
  vsnprintf(s_format_buffer, sizeof(s_format_buffer), format, args);
  va_end(args);

  //screen_set_cursor(row, col);

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


void screen_page_handle(screen_page_t* win, int key)
{
  int page = win->current_page;
  int new_offset = 0;
  int total_items = 0;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;  
  
  //행의 개수를 화면 행의 배수로 만든다.
  total_items = ALIGN_UP(win->total_items[page], view_rows);

  switch (key)
  {
    case KEY_CODE_UP:  // 위로 스크롤 
      if (win->scroll_offset[page] > 0)
      {
        if(win->chunk_scroll_enable)//화면 전체행단위로 스크롤이라면
        {
          win->scroll_offset[page] -= view_rows; //화면 
        }
        else
        {
          win->scroll_offset[page] -= 1;// 1개행씩 스크롤
        }

        if (win->scroll_offset[page] < 0)
        {
          win->scroll_offset[page] = 0;
        }
      }
      break;
    case KEY_CODE_DOWN:  // 아래로 스크롤
      if(win->chunk_scroll_enable)
      {
        new_offset = win->scroll_offset[page] + view_rows;
      }
      else
      {
        new_offset = win->scroll_offset[page] + 1;
      }

      if (new_offset < total_items)
      {
        win->scroll_offset[page] = new_offset;
      }
      break;
    case KEY_CODE_LEFT:  // 이전 페이지
      if (win->multi_page_enable && (win->current_page > 0))
      {
        win->current_page--;
      }
      break;
    case KEY_CODE_RIGHT:  // 다음 페이지
      if (win->multi_page_enable)
      {
        if (win->current_page < win->total_pages - 1)
        {
          win->current_page++;
        }
      }
      break;
  }
}



void screen_menu_printf(screen_menu_t *win,int index,const char *format, ...)
{
  char s_format_buffer[MAX_COLS]; // 정적 버퍼 크기는 필요에 따라 조정
  char selection_indicator;
  int display_row;
  int i;
  int title_offset = 0;
  int row_index;
  va_list args;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;
  

  row_index = win->total_items;

  win->index_list[row_index] = index;
   win->total_items++;

   if (win->title != NULL)
   {
     if (win->current_row == 0)
     {
        screen_printf(0,0,win->title);
        win->current_row++;
     }
     title_offset = 1;
   }

  if (win->current_row >= view_rows)
  {
    return;
  }

  // 가변 인자를 문자열로 포맷팅
  va_start(args, format);
  vsnprintf(s_format_buffer, sizeof(s_format_buffer), format, args);
  va_end(args);


  // 타이틀 오프셋을 고려하여 스크롤 범위 조정
  if (row_index >= win->scroll_offset && row_index < win->scroll_offset + view_rows - title_offset)
  {
    display_row = row_index - win->scroll_offset + title_offset;

//    screen_set_cursor(display_row, 0);

    // 선택된 항목이면 '*', 아니면 ' ' 표시
    if(win->enter_long_key_active==true)
    {
      selection_indicator = (win->selected_index == row_index) ? '>' : ' ';
    }
    else
    {
      selection_indicator = (win->selected_index == row_index) ? '*' : ' ';
    }
    screen_put_ch(display_row, 0, selection_indicator);
    screen_printf(display_row, 1, "%s", s_format_buffer);

    win->current_row++;
  }

  if (row_index >= win->total_items)
  {
    win->total_items = row_index + 1;
  }
}


void screen_menu_handle(screen_menu_t* win, int key)
{
  int title_offset = (strlen(win->title) > 0) ? 1 : 0;// 타이틀이 존재 하면 
  int effective_view_row ;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;
  
  
     effective_view_row = view_rows - title_offset;// 타이틀을 제외한 행만 유효한 표시행 
    
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



void screen_menu_start(screen_menu_t *win)
{
  win->current_row = 0;
  win->total_items = 0;
}

void screen_menu_clear(screen_menu_t *win)
{
  int i;
  int display_row;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;
  
  
  for (display_row = win->current_row; display_row < view_rows; display_row++)
  {
    for (i = 0; i < view_cols; i++)
    {
      screen_put_ch(display_row, i, ' ');
    }
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





/**
 * @brief win->current_row기준으로 view_row 남은 행을 전부 공백표시,clear
 */
void screen_page_clear(screen_page_t *win)
{
  int i;
  int display_row;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;  
  
  
  for (display_row = win->current_row; display_row < view_rows; display_row++)
  {
    for (i = 0; i < view_cols; i++)
    {
      screen_put_ch(display_row, i, ' ');
    }
  }
}

void screen_page_start(screen_page_t *win)
{
  int page = win->current_page;
  win->current_row  = 0;
  win->total_items[page] = 0;
}

void screen_page_printf(screen_page_t *win,const char *format, ...)
{
  char s_format_buffer[MAX_COLS + 1]; // 정적 버퍼 크기는 필요에 따라 조정
  int display_row;

  int i;
  int text_len;
  int page;
  int row_index;
  va_list args;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;  
  
  
  row_index = win->total_items[win->current_page];
  
  win->total_items[win->current_page]++;

  if (win->current_row >= view_rows)
  {
    return;
  }

  // 가변 인자를 문자열로 포맷팅
  va_start(args, format);
  vsnprintf(s_format_buffer, sizeof(s_format_buffer), format, args);
  va_end(args);


  page = win->current_page;

  if (row_index >= win->scroll_offset[page] && row_index < win->scroll_offset[page] + view_rows)
  {
    display_row = row_index - win->scroll_offset[page];

    //screen_set_cursor(display_row, 0);

    screen_printf(display_row,0,s_format_buffer);

    win->current_row++;
  }
}

