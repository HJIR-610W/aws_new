#include "view_driver.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#include "app_key.h"
#include "debug_io.h"
#include "console_utile.h"
#include "util_stdio.h"
#include "user_heap.h"

static layout_t g_layout = {1, 1, 150, 0};//화면 전체 정보



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
	
	for (int i = 0; i < WIN_PAGE_MAX; i++) {
		win->scroll_offset[i] = 0;
		win->total_items[i] = 0;
		win->selected_item[i] = 0;
	}
}
#define OUT_BUFF_SIZE 1024
#define HLINE_BUFF_SIZE 300
#define TITLE_BUFF_SIZE 300
//실시간 값 표시
void win_printf_title(win_t* win, const char* pFmt, ...)
{
  char buffer[256];
  va_list ap;
  int len;
  int remain_len;
  const char* ansi_begin = "";
  const char* ansi_end = "";



//  snprintf(&buffer[len], sizeof(buffer) - len, "\x1B[%d;%dH┌%s┐",win->start_y, win->start_x);  
  /* 1) 가로선(─) 문자열 생성: view_col-2 만큼 */
  /* 상단 라인 출력: ┌───┐ */
 len = 0;

 len =  snprintf(&buffer[len], sizeof(buffer) - len, "\x1B[%d;%dH┌",win->start_y, win->start_x);

  for (int i = 0; i < win->view_col - 2; i++)
  {
    len += snprintf(&buffer[len], sizeof(buffer) - len, "─");
    if (len >= (int)sizeof(buffer) - 1)
    {
      break;
    }
  }

  snprintf(&buffer[len], sizeof(buffer) - len, "┐");
  debug_printf("%s", buffer);



  /* 2) 타이틀 문자열 1회 생성 */
  /* 4) 포커스/선택 스타일 결정 */
    /* 6) 타이틀 라인 출력: │title│ */
  if (win->is_focused)
  {
    ansi_begin = "\x1B[32m";
    ansi_end = "\x1B[0m";
  }
  else if (win->is_selected)
  {
    ansi_begin = "\x1B[7m";
    ansi_end = "\x1B[0m";
  }


  len = 0;
   debug_printf("\x1B[%d;%dH│%s",win->start_y + 1, win->start_x,ansi_begin);

  va_start(ap, pFmt);
  len += vsnprintf(&buffer[len], sizeof(buffer) - len, pFmt, ap);
  va_end(ap);

  if (len < 0)
  {
    len = 0;
    buffer[0] = '\0';
  }
  if (len >= (int)sizeof(buffer))
  {
    len = (int)sizeof(buffer) - 1;
    buffer[len] = '\0';
  }

  if (win->total_pages > 1)
  {
    len += snprintf(&buffer[len], sizeof(buffer) - len," [%d/%d]", win->current_page + 1, win->total_pages);
    if (len >= (int)sizeof(buffer))
    {
      len = (int)sizeof(buffer) - 1;
      buffer[len] = '\0';
    }
  }

  remain_len = win->view_col - utf8_strlen(buffer) - 2;
  if (remain_len < 0)
  {
    remain_len = 0;
  }

  for (int i = 0; i < remain_len; i++)
  {
    len += snprintf(&buffer[len], sizeof(buffer) - len, " ");
    if (len >= (int)sizeof(buffer) - 1)
    {
      break;
    }
  }

  debug_printf("%s", buffer);
  debug_printf("%s│", ansi_end);


  /* 7) 구분선 라인 출력: ├───┤ */
  len = 0;
  len = snprintf(&buffer[len], sizeof(buffer) - len, "\x1B[%d;%dH├",win->start_y + 2, win->start_x);
  
  for (int i = 0; i < win->view_col - 2; i++)
  {
    len += snprintf(&buffer[len], sizeof(buffer) - len, "─");
    if (len >= (int)sizeof(buffer) - 1)
    {
      break;
    }
  }
  snprintf(&buffer[len], sizeof(buffer) - len, "┤");
  debug_printf("%s", buffer);


}



void win_print_close(win_t* win)
{
  char buff[256];
  int offset = 0;

  /* 커서 이동 + 좌측 하단 모서리 */
  offset += snprintf(
    buff + offset,
    sizeof(buff) - offset,
    "\x1B[%d;%dH└",
    win->start_y + 3 + win->current_row,
    win->start_x
  );

  /* 하단 가로선 */
  for (int i = 0; i < win->view_col - 2; i++)
  {
    offset += snprintf(
      buff + offset,
      sizeof(buff) - offset,
      "─"
    );
  }

  /* 우측 하단 모서리 + 개행 */
  offset += snprintf(
    buff + offset,
    sizeof(buff) - offset,
    "┘\n"
  );

  debug_printf("%s", buff);
}

/**
 * @brief 창에 행에 문자열 출력
 * @param win 창정보
 * @param row_index 창의 행 정보
 */
void win_printf_row(win_t* win, int row_index, const char* pFmt, ...)
{
  char line[300];
  char out[360];
  va_list ap;
  int len;
  int page;
  int remain_len;

  if (win->current_row >= win->view_row)
  {
    return;
  }

  page = win->current_page;

  if (row_index < win->scroll_offset[page] ||
      row_index >= (win->scroll_offset[page] + win->view_row))
  {
    return;
  }

  /* 1) 라인 내용 생성: "│" + formatted + padding + "│" */
  len = 0;
  len += snprintf(&line[len], sizeof(line) - len, "│");

  va_start(ap, pFmt);
  len += vsnprintf(&line[len], sizeof(line) - len, pFmt, ap);
  va_end(ap);

  if (len < 0)
  {
    len = 0;
    line[0] = '\0';
  }
  if (len >= (int)sizeof(line))
  {
    len = (int)sizeof(line) - 1;
    line[len] = '\0';
  }

  /* view_col 기준(문자 수)으로 오른쪽 패딩: 마지막 우측 테두리 1칸 남김 */
  remain_len = win->view_col - utf8_strlen(line) - 1;
  if (remain_len < 0)
  {
    remain_len = 0;
  }

  for (int i = 0; i < remain_len; i++)
  {
    len += snprintf(&line[len], sizeof(line) - len, " ");
    if (len >= (int)sizeof(line) - 1)
    {
      break;
    }
  }

  len += snprintf(&line[len], sizeof(line) - len, " │");

  /* 2) 커서 이동까지 포함해서 한 번에 출력 */
  snprintf(out, sizeof(out),
           "\x1B[%d;%dH%s",
           win->start_y + 3 + win->current_row,
           win->start_x,
           line);

  debug_printf("%s", out);

  win->current_row++;
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

  key = debug_get_key(timeout_ms);
  return key;
}


void calculate_window_position(win_t* win)
{
	int win_height = win->view_row+3;//타이틀 3행 

	if (g_layout.next_x + win->view_col > g_layout.max_width)
	{
		g_layout.next_x = 1;
		g_layout.next_y += g_layout.row_height + 1;
		g_layout.row_height = 0;
	}
	
	win->start_x = g_layout.next_x;
	win->start_y = g_layout.next_y;

	g_layout.next_x += win->view_col + 2;
	if (win_height > g_layout.row_height)
	{
		g_layout.row_height = win_height;
	}
}

void reset_layout(void)
{
	g_layout.next_x = 1;
	g_layout.next_y = 1;
	g_layout.row_height = 0;
}