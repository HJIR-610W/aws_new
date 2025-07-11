#include "view_driver.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#include "app_key.h"
#include "dev_io.h"
#include "console_utile.h"
#include "util_stdio.h"
static layout_t g_layout = {1, 1, 125, 0};



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
	int len;


	if (win->current_row >= win->view_row)
		return;

	io_printf("\x1B[%d;%dH", win->start_y + 3 + win->current_row, win->start_x);
	va_start(ap, pFmt);


	buff[0] = '|';
	len = 1;
	len += vsnprintf((char*)&buff[1], sizeof(buff) - 2, (char*)pFmt, ap);
	va_end(ap);

		io_printf(buff);
		len = win->view_col - utf8_strlen(buff)-1;
		for (int i = 0; i < len; i++) io_printf(" ");
		io_printf("|\r\n");


	win->current_row++;
}

void win_printf_title(win_t* win, const char* pFmt, ...)
{
	char buff[150];
	va_list ap;
	int len=0;
  int remain_len;


	io_printf("\x1B[%d;%dH", win->start_y, win->start_x);


	//상단 +----+ 출력
	io_printf("+");
	for (int i = 0; i < win->view_col - 2; i++) {
		io_printf("-");
	}
	io_printf("+\n");

	//타이틀 출력
	io_printf("\x1B[%d;%dH", win->start_y + 1, win->start_x);

	va_start(ap, pFmt);

	len += vsnprintf(&buff[0], sizeof(buff) - 2, pFmt, ap);
	va_end(ap);
	
	if (win->total_pages > 1)
	{
    len+=snprintf(&buff[len], sizeof(buff)-len, " [%d/%d]", win->current_page + 1, win->total_pages);
	}

  remain_len = win->view_col - utf8_strlen(buff)  -2;

  for (int i = 0; i < remain_len; i++)
  {
    buff[len++] = ' ';
  }

  buff[len++] = '\0';

	if (win->is_focused)
	{
		io_printf("|\x1B[32m%s\x1B[0m|\r\n",  &buff[0]);
	}
	else if (win->is_selected)
	{
		io_printf("|\x1B[7m%s\x1B[0m|\r\n",  &buff[0]);
	} else
	{
    io_printf("|%s|\r\n", buff);
  }

	io_printf("\x1B[%d;%dH", win->start_y + 2, win->start_x);
	io_printf("+");
	for (int i = 0; i < win->view_col - 2; i++) {
		io_printf("-");
	}
	io_printf("+\n");
}

void win_print_close(win_t* win)
{
	io_printf("\x1B[%d;%dH", win->start_y + 3 + win->current_row, win->start_x);
	io_printf("+");
	for (int i = 0; i < win->view_col - 2; i++) {
		io_printf("-");
	}
	io_printf("+\n");
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

int view_get_key_input(uint32_t timeout_ms)
{
  int key;

  key = get_key(timeout_ms);
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
	
	g_layout.next_x += win_width + 2;
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