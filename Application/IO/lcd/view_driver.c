#include "view_driver.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "cli_key_code.h"

#include "dev_io.h"



static layout_t g_layout = {1, 1, 125, 0};


int lcd_printf(char const* const _Format, ...)
{
	va_list args;
	int result;
	
	va_start(args, _Format);
        result = io_vprintf(_Format, args);
        va_end(args);
	
	return result;
}

layout_t* get_layout(void)
{
	return &g_layout;
}

void create_win(win_t* win, int start_x, int start_y, int view_row, int view_col)
{
	win->start_x = start_x;
	win->start_y = start_y;
	
	win->current_row = 0;
	win->view_row = view_row;
	win->view_col = view_col;
	win->is_focused = 0;
	win->is_selected = 0;
	win->current_page = 0;
	win->total_pages = 1;
	
	for (int i = 0; i < 10; i++) {
		win->scroll_offset[i] = 0;
		win->total_items[i] = 0;
		win->selected_item[i] = 0;
	}
}

void win_printf(win_t* win, const char* pFmt, ...)
{
	char buff[150];
	va_list ap;

	if (win->current_row >= win->view_row)
		return;

	lcd_printf("\x1B[%d;%dH", win->start_y + 3 + win->current_row, win->start_x);
	va_start(ap, pFmt);
	int len;
	buff[0] = '|';
	len = 1;
	len += vsnprintf((char*)&buff[1], sizeof(buff) - 2, (char*)pFmt, ap);
	va_end(ap);

	for (int i = len; i < win->view_col - 1; i++) {
		buff[i] = ' ';
	}
	buff[win->view_col - 1] = '|';
	buff[win->view_col] = '\0';
	lcd_printf("%s\n", buff);

	win->current_row++;
}

void win_printf_title(win_t* win, const char* pFmt, ...)
{
	char buff[150];
	va_list ap;

	lcd_printf("\x1B[%d;%dH", win->start_y, win->start_x);

	lcd_printf("+");
	for (int i = 0; i < win->view_col - 2; i++) {
		lcd_printf("-");
	}
	lcd_printf("+\n");

	lcd_printf("\x1B[%d;%dH", win->start_y + 1, win->start_x);
	va_start(ap, pFmt);
	int len = 1;
	buff[0] = '|';
	len += vsnprintf(&buff[1], sizeof(buff) - 2, pFmt, ap);
	va_end(ap);
	
	if (win->total_pages > 1) {
		char page_info[20];
		snprintf(page_info, sizeof(page_info), " [%d/%d]", win->current_page + 1, win->total_pages);
		int page_len = (int)strlen(page_info);
		for (int i = 0; i < page_len && len + i < win->view_col - 1; i++) {
			buff[len + i] = page_info[i];
		}
		len += page_len;
	}

	for (int i = len; i < win->view_col - 1; i++) {
		buff[i] = ' ';
	}
	buff[win->view_col - 1] = '|';
	buff[win->view_col] = '\0';

	if (win->is_focused) {
		lcd_printf("|\x1B[32m%.*s\x1B[0m|\n", win->view_col - 2, &buff[1]);
	} else if (win->is_selected) {
		lcd_printf("|\x1B[7m%.*s\x1B[0m|\n", win->view_col - 2, &buff[1]);
	} else {
		lcd_printf("%s\n", buff);
	}

	lcd_printf("\x1B[%d;%dH", win->start_y + 2, win->start_x);
	lcd_printf("+");
	for (int i = 0; i < win->view_col - 2; i++) {
		lcd_printf("-");
	}
	lcd_printf("+\n");
}

void win_print_close(win_t* win)
{
	lcd_printf("\x1B[%d;%dH", win->start_y + 3 + win->current_row, win->start_x);
	lcd_printf("+");
	for (int i = 0; i < win->view_col - 2; i++) {
		lcd_printf("-");
	}
	lcd_printf("+\n");
}

void win_print_row(win_t* win, int row_index, const char* buff)
{
	if (win->current_row >= win->view_row)
		return;
	
	int page = win->current_page;
	if (row_index >= win->scroll_offset[page] && row_index < win->scroll_offset[page] + win->view_row)
	{
		win_printf(win, "%s", buff);
	}
}

void handle_scroll(win_t* win, int key)
{
	if (!win->is_selected) return;
	
	int page = win->current_page;
	switch (key) {
	case KEY_UP: // Up arrow - 페이지 단위 스크롤
		if (win->scroll_offset[page] > 0) {
			win->scroll_offset[page] -= win->view_row;
			if (win->scroll_offset[page] < 0) {
				win->scroll_offset[page] = 0;
			}
			win->selected_item[page] = win->scroll_offset[page];
		}
		break;
	case KEY_DOWN: // Down arrow - 페이지 단위 스크롤
		if (win->scroll_offset[page] + win->view_row < win->total_items[page]) {
			win->scroll_offset[page] += win->view_row;
			if (win->scroll_offset[page] + win->view_row > win->total_items[page]) {
				win->scroll_offset[page] = win->total_items[page] - win->view_row;
				if (win->scroll_offset[page] < 0) {
					win->scroll_offset[page] = 0;
				}
			}
			win->selected_item[page] = win->scroll_offset[page];
		}
		break;
	case KEY_LEFT: // Left arrow
		if (win->current_page > 0) {
			win->current_page--;
		}
		break;
	case KEY_RIGHT: // Right arrow
		if (win->current_page < win->total_pages - 1) {
			win->current_page++;
		}
		break;
	}
}


void handle_navigation_ptr(win_t** windows, int win_count, int* current_win, int key)
{

	switch (key) {
	case KEY_LEFT: // Left arrow
		windows[*current_win]->is_focused = 0;
		*current_win = (*current_win - 1 + win_count) % win_count;
		windows[*current_win]->is_focused = 1;
		break;
	case KEY_RIGHT: // Right arrow
		windows[*current_win]->is_focused = 0;
		*current_win = (*current_win + 1) % win_count;
		windows[*current_win]->is_focused = 1;
		break;
	}
}


int get_key_input(uint32_t timeout_ms)
{
  int key = get_key(timeout_ms);

  return key;
}


void calculate_window_position(win_t* win, int win_width, int win_height)
{
	if (g_layout.next_x + win_width > g_layout.max_width) {
		g_layout.next_x = 1;
		g_layout.next_y += g_layout.row_height + 1;
		g_layout.row_height = 0;
	}
	
	win->start_x = g_layout.next_x;
	win->start_y = g_layout.next_y;
	
	g_layout.next_x += win_width + 1;
	if (win_height > g_layout.row_height) {
		g_layout.row_height = win_height;
	}
}

void reset_layout(void)
{
	g_layout.next_x = 1;
	g_layout.next_y = 1;
	g_layout.row_height = 0;
}