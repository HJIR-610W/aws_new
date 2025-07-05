#include "lcd_driver.h"
#include <string.h>
#include <stdio.h>

#include "app_button.h"
#include "app_lcd.h"

#include "cli_key_code.h"
void lcd_clear(void)
{
	clcd_clear();
}

void lcd_set_cursor(int row, int col)
{
	clcd_set_position(row,col);
}

void lcd_write_char(char c)
{
	char buff[2]={0,0};

	buff[0] = c;

	clcd_write_string(buff);
}

void lcd_write_string(const char* str)
{
	clcd_write_string(str);
}

// LCD 윈도우 관리 함수들
void lcd_create_win(lcd_win_t* win)
{
	win->current_row = 0;
	win->view_row = LCD_ROWS;
	win->view_col = LCD_COLS;
	win->current_page = 0;
	win->total_pages = 1;
	
	for (int i = 0; i < LCD_PAGE_MAX; i++) {
		win->scroll_offset[i] = 0;
		win->total_items[i] = 0;
	}
}

void lcd_clear_win(lcd_win_t* win)
{
	lcd_clear();
	win->current_row = 0;
}

void lcd_print_row(lcd_win_t* win, int row_index, const char* text)
{
	if (win->current_row >= win->view_row)
		return;
	
	int page = win->current_page;
	
	// 스크롤 범위 체크
	if (row_index >= win->scroll_offset[page] && 
		row_index < win->scroll_offset[page] + win->view_row) {
		
		int display_row = row_index - win->scroll_offset[page];
		
		// 커서를 해당 행으로 이동
		lcd_set_cursor(display_row, 0);
		
		// 텍스트를 LCD 너비에 맞게 조정
		char display_text[LCD_COLS + 1];
		memset(display_text, ' ', LCD_COLS);
		display_text[LCD_COLS] = '\0';
		
		// 텍스트 복사 (LCD 너비 초과하면 잘림)
		int text_len = strlen(text);
		int copy_len = (text_len > LCD_COLS) ? LCD_COLS : text_len;
		memcpy(display_text, text, copy_len);
		
		// LCD에 출력
		lcd_write_string(display_text);
		
		win->current_row++;
	}
}

void lcd_refresh_win(lcd_win_t* win)
{
	// LCD 전체 새로고침
	lcd_clear();
	win->current_row = 0;
}

void lcd_handle_scroll(lcd_win_t* win, int key)
{
	int page = win->current_page;
	
	switch (key) {
          case KEY_CODE_UP:  // 위로 스크롤
            if (win->scroll_offset[page] > 0)
            {
              win->scroll_offset[page] -= win->view_row;
              if (win->scroll_offset[page] < 0)
              {
                win->scroll_offset[page] = 0;
              }
            }
            break;

          case KEY_CODE_DOWN:  // 아래로 스크롤
            if (win->scroll_offset[page] + win->view_row < win->total_items[page])
            {
              win->scroll_offset[page] += win->view_row;
              if (win->scroll_offset[page] + win->view_row > win->total_items[page])
              {
                win->scroll_offset[page] = win->total_items[page] - win->view_row;
                if (win->scroll_offset[page] < 0)
                {
                  win->scroll_offset[page] = 0;
                }
              }
            }
            break;

          case KEY_CODE_LEFT:  // 이전 페이지
            if (win->current_page > 0)
            {
              win->current_page--;
            }
            break;

          case KEY_CODE_RIGHT:  // 다음 페이지
            if (win->current_page < win->total_pages - 1)
            {
              win->current_page++;
            }
            break;
	}
}

int lcd_get_key_input(void)
{
	int key;

	key = get_button_key(1000);

	return key;

}