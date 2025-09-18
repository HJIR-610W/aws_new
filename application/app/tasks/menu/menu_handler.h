

#ifndef MENU_HANDLER_H

#define MENU_HANDLER_H
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "console_define.h"
#include "app_screen.h"

#define LCD_ROWS 8
#define LCD_COLS 21

#define SCREEN_PAGE_MAX 10

typedef int32_t menu_status_t;

typedef struct
{
  bool multi_page_enable;   // 화면은 left,right 버튼으로 이동가능한 멀티 페이지로 구성됨
  bool chunk_scroll_enable; // 화면은 up,down 버튼으로 스크롤할때 화면 전체 스크롤할지, 행스크롤할지
  uint8_t total_pages;      // 총 몇개의 페이지로 구성된건지
  screen_instance_t *p_screen;
  uint8_t current_row;                   // 현재의 행번호 (0부터 시작)
  int8_t scroll_offset[SCREEN_PAGE_MAX]; // 개별 페이지 up,down 버튼으로 이동된 오프셋
  uint8_t total_items[SCREEN_PAGE_MAX];  // 페이지에 몇개의 행이 있는지
  uint8_t current_page;                  // 현재 페이지 번호
} screen_page_t;                         // 정보 표현 페이지 구현시 사용

typedef struct
{
  char title[21 + 1];
  bool enter_long_key_active; // 해당 행이 엔터롱키기능이 있는지
  screen_instance_t *p_screen;
  uint8_t current_row;    // 현재의 로우수
  int8_t scroll_offset;   // 현재 row의 오프셋
  uint8_t total_items;    // 행의 총 갯수
  uint8_t selected_index; // 별표가 위치한 곳의 행 번호
  uint8_t index_list[50]; // 메뉴 번호
} screen_menu_t;          // 메뉴 페이지 구현시 사용, 항상 앞에 *,>표시됨


typedef struct string_fmt_s
{
  char data[21];
  const char* fmt;
} string_fmt_t;

void screen_page_create(screen_page_t *win);
void screen_page_handle(screen_page_t *win, int key);
void screen_page_clear(screen_page_t *win);
void screen_page_start(screen_page_t *win);
void screen_page_printf(screen_page_t *win, const char *format, ...);

void screen_menu_create(screen_menu_t *win, const char *titile);
void screen_menu_handle(screen_menu_t *win, int key);
void screen_menu_clear(screen_menu_t *win);
void screen_menu_start(screen_menu_t *win);
void screen_menu_printf(screen_menu_t *win, int index, const char *format, ...);

menu_status_t input_fmt(string_fmt_t *strfmt, const char *title);
menu_status_t input_decimal(const char *title, int min, int max, int *val);
menu_status_t input_float(const char *title, float min, float max, float *val, const char *fmt);
menu_status_t input_combobox(const char *title, const char *item_list[], int32_t item_count, int *choice);
menu_status_t input_active(const char *title, int32_t *choice);
menu_status_t input_password(const char *title, int32_t *password);
menu_status_t show_popup(const char *title, const char *message);


int32_t make_sreen_row(char *buff, const char *pFmt, ...);
int32_t get_menu_key(uint32_t timeout_ms);

int32_t convert_key_to_status(int key);

void register_key_callback(void (*callback)(void));
void unregister_key_callback(void);

#endif