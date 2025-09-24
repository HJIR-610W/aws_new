#ifndef APP_SCREEN_H
#define APP_SCREEN_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>




typedef struct screen_instance
{
  int32_t width_pixel; //가로 픽셀
  int32_t height_pixcel;//세로 픽셀
  uint8_t font_rows;//캐릭터 모드인경우 행수
  uint8_t font_cols;//캐릭터 모드인경우 열수
  bool screen_on;//현재 화면 꺼졌는지 켜졌는지 상태
  bool graphic_mode;//그래픽 모드인지 캐릭터 모드인지
} screen_instance_t;


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
void screen_off(void);
void screen_on(void);
void screen_puts(int row,int col,const char *string);

#endif