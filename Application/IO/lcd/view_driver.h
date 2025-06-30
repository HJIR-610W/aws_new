#ifndef VIEW_DRIVER_H
#define VIEW_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "cli_key_code.h"

#define KEY_BREAK KEY_CODE_CTRL_Q
#define KEY_ENTER KEY_CODE_ENTER
#define KEY_UP KEY_CODE_UP
#define KEY_DOWN KEY_CODE_DOWN
#define KEY_LEFT KEY_CODE_LEFT
#define KEY_RIGHT KEY_CODE_RIGHT

#define ALIGN_UP(val, align)  (((val) + ((align) - 1)) / (align) * (align))


#define WIN_PAGE_MAX 5
typedef struct
{
	int start_x;
	int start_y;
	int current_row;
	int view_row;
	int view_col;
	int scroll_offset[WIN_PAGE_MAX];
	int total_items[WIN_PAGE_MAX];
	int selected_item[WIN_PAGE_MAX];
	int is_focused;
	int is_selected;
	int current_page;
	int total_pages;
} win_t;



typedef enum {
	MODE_NAVIGATE,
	MODE_SELECT
} app_mode_t;

typedef struct {
	int next_x;      // 다음 윈도우가 배치될 X 좌표 (가로 위치)
	int next_y;      // 다음 윈도우가 배치될 Y 좌표 (세로 위치)
	int max_width;   // 화면의 최대 너비 (줄바꿈 기준)
	int row_height;  // 현재 행의 최대 높이 (세로 정렬용)
} layout_t;

// Window management functions
void create_win(win_t* win, int start_x, int start_y, int view_row, int view_col);
void win_printf(win_t* win, const char* pFmt, ...);
void win_printf_title(win_t* win, const char* pFmt, ...);
void win_print_close(win_t* win);
void win_print_row(win_t* win, int row_index, const char* buff);

// Scroll and navigation functions
void handle_scroll(win_t* win, int key);
void handle_navigation_ptr(win_t** windows, int win_count, int* current_win, int key);

// Layout functions
void calculate_window_position(win_t* win, int win_width, int win_height);
void reset_layout(void);

// Input function
int get_key_input(uint32_t timeout_ms);

// Global layout accessor
layout_t* get_layout(void);

#ifdef __cplusplus
}
#endif

#endif // VIEW_DRIVER_H