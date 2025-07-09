#ifndef APP_SCREEN_H
#define APP_SCREEN_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "driver_lcd.h"

void clcd_printf(int row, int col, char const* const _Format, ...);
void screen_home(void);
void screen_display_on(void);
void screen_display_off(void);
void screen_set_cursor(int row, int col);

void clcd_write_string_at(int row, int col, const char *str);

void screen_set_mode(eLCD_MODE_t lcd_mode);
void screen_set_pixel( uint8_t x, uint8_t y, bool on);
void screen_refresh(void);
void screen_put_ch(int row, int col, uint8_t ch);


#define SCREEN_PAGE_MAX 10

typedef enum
{
  SCREEN_STATE_OFF,
  SCREEN_STATE_ON
} eSCREEN_STATE_t;

typedef struct
{
  int current_row;
  int view_row;
  int view_col;
  int scroll_offset[SCREEN_PAGE_MAX];
  int total_items[SCREEN_PAGE_MAX];
  int current_page;
  int total_pages;
} screen_page_t;

void screen_init(void);
void screen_clear(screen_page_t* p_win);
void screen_page_create(screen_page_t* win, int rows, int cols);
void screen_printf_row(screen_page_t* win, int row_index, const char* format, ...);
void screen_clear_row(screen_page_t* win, int row_index);
void screen_handle_scroll(screen_page_t* win, int key);
void screen_off(screen_page_t* p_screen);
void screen_on(screen_page_t* p_screen);
#endif