#ifndef APP_SCREEN_H
#define APP_SCREEN_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "driver_lcd.h"

typedef struct screen_instance
{
  int32_t width_pixel;
  int32_t height_pixcel;
  int8_t font_w;
  int8_t font_h;
  int8_t font_rows;
  int8_t font_cols;
} screen_instance_t;

screen_instance_t* screen_get_instance(void);

void screen_home(void);
void screen_display_on(void);
void screen_display_off(void);
void screen_set_cursor(int row, int col);



void screen_set_mode(eLCD_MODE_t lcd_mode);
void screen_set_pixel( uint8_t x, uint8_t y, bool on);
void screen_refresh(void);
void screen_put_ch(int row, int col, uint8_t ch);
void screen_printf(int row, int col, const char* format, ...);

#define SCREEN_PAGE_MAX 10

    typedef enum {
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
  int multi_page_use;
  int chunk_scroll_use;
} screen_page_t;


typedef struct
{
  char title[16+1];
  int current_row;
  int view_row;
  int view_col;
  int scroll_offset;
  int total_items;
  int selected_index;
  uint8_t index_list[10];
} screen_menu_t;



void screen_init(void);
void screen_clear(void);
void screen_page_create(screen_page_t* win, int rows, int cols);
void screen_printf_row(screen_page_t* win, int row_index, const char* format, ...);
void screen_clear_row(screen_page_t* win, int row_index);
void screen_handle_scroll(screen_page_t* win, int key);
void screen_menu_handle(screen_menu_t* win, int key);
void screen_off(void);
void screen_on(void);
void screen_menu_create(screen_menu_t* win, int rows, int cols);
void screen_menu_clear(screen_menu_t* win);
void screen_menu_printf_row(screen_menu_t* win, int row_index, const char* format, ...);
void screen_menu_clear_row(screen_menu_t* win, int row_index);
void screen_update_list(screen_menu_t* p_screen, int index, int id);
void screen_clear_unsued_line(screen_menu_t* p_win);
#endif