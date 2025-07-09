#ifndef SCREEN_DRIVER_H
#define SCREEN_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
  
#define SCREEN_PAGE_MAX 10

typedef struct
{
	int current_row;
	int view_row;
	int view_col;
	int scroll_offset[SCREEN_PAGE_MAX];
	int total_items[SCREEN_PAGE_MAX];
	int current_page;
	int total_pages;
} screen_t;

void screen_create(screen_t* win,int rows,int cols);
void screen_print_row(screen_t* win, int row_index, const char* text);
void screen_printf_row(screen_t* win, int row_index, const char* format, ...);
void screen_clear_row(screen_t* win, int row_index);
void screen_handle_scroll(screen_t* win, int key);


#ifdef __cplusplus
}
#endif

#endif