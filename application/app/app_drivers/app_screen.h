#ifndef APP_SCREEN_H
#define APP_SCREEN_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#define SCREEN_PAGE_MAX 10

//화면의 기본 정보
typedef struct screen_instance
{
  int32_t width_pixel; //가로 픽셀
  int32_t height_pixcel;//세로 픽셀
  uint8_t font_rows;//캐릭터 모드인경우 행수
  uint8_t font_cols;//캐릭터 모드인경우 열수
  bool screen_on;//현재 화면 꺼졌는지 켜졌는지 상태
  bool graphic_mode;//그래픽 모드인지 캐릭터 모드인지
} screen_instance_t;

typedef struct
{
  bool multi_page_enable;   // 화면은 left,right 버튼으로 이동가능한 멀티 페이지로 구성됨
  bool chunk_scroll_enable; // 화면은 up,down 버튼으로 스크롤할때 화면 전체 스크롤할지, 행스크롤할지
  uint8_t total_pages;      // 총 몇개의 페이지로 구성된건지
  screen_instance_t *p_screen;
  uint8_t current_row; // 현재의 행번호 (0부터 시작)
  int8_t scroll_offset[SCREEN_PAGE_MAX]; // 개별 페이지 up,down 버튼으로 이동된 오프셋
  uint8_t total_items[SCREEN_PAGE_MAX];  // 페이지에 몇개의 행이 있는지
  uint8_t current_page;                  // 현재 페이지 번호
} screen_page_t;//정보 표현 페이지 구현시 사용

typedef struct
{
  char title[21 + 1];
  bool enter_long_key_active; // 해당 행이 엔터롱키기능이 있는지
  screen_instance_t *p_screen;
  uint8_t current_row; // 현재의 로우수
  int8_t scroll_offset; // 현재 row의 오프셋
  uint8_t total_items;   // 행의 총 갯수
  uint8_t selected_index; // 별표가 위치한 곳의 행 번호
  uint8_t index_list[50]; // 메뉴 번호
} screen_menu_t;//메뉴 페이지 구현시 사용, 항상 앞에 *,>표시됨


screen_instance_t* screen_get_instance(void);

void screen_init(void);
void screen_home(void);
void screen_display_on(void);
void screen_display_off(void);
void screen_set_cursor(int row, int col);
void screen_set_pixel( uint8_t x, uint8_t y, bool on);
void screen_refresh(void);
void screen_put_ch(int row, int col, uint8_t ch);
void screen_printf(int row, int col, const char* format, ...);
void screen_clear(void);

void screen_page_create(screen_page_t *win);
void screen_page_handle(screen_page_t *win, int key);
void screen_page_clear(screen_page_t *win);
void screen_page_start(screen_page_t *win);
void screen_page_printf(screen_page_t *win, const char *format, ...);

void screen_menu_create(screen_menu_t *win, const char *titile);
void screen_menu_handle(screen_menu_t* win, int key);
void screen_menu_clear(screen_menu_t *win);
void screen_menu_start(screen_menu_t *win);
void screen_menu_printf(screen_menu_t *win, int index, const char *format, ...);

void screen_off(void);
void screen_on(void);

void screen_widget_progress(int row, int col, float percentage);

#endif