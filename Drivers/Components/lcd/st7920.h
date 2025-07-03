

#ifndef ST7920_H
#define ST7920_H

#include <stdint.h>
#include <stdbool.h>

#include "driver_interface.h"


driver_t* st7920_open(void);
void st7920_reset(driver_t *drv);
void st7920_send_cmd(driver_t *drv, uint8_t cmd);
void st7920_send_data(driver_t *drv, uint8_t data);
void st7920_send_byte(driver_t *drv, uint8_t sync, uint8_t data);


void st7920_clear_screen(driver_t *drv);
void st7920_home(driver_t *drv);
void st7920_display_on(driver_t *drv);
void st7920_display_off(driver_t *drv);

/* 모드 및 텍스트 */
void st7920_set_graphic_mode(driver_t *drv, bool enable);
void st7920_set_position(driver_t *drv, uint8_t row, uint8_t col);
void st7920_write_string(driver_t *drv, int row, int col, const char *str);
void st7920_write_string_simple(driver_t *drv, const char *str);

/* 그래픽 출력 - 그래픽 모드에서만 동작 */
void st7920_set_pixel(driver_t *drv, uint8_t x, uint8_t y, bool on);
void st7920_draw_line(driver_t *drv, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool on);
void st7920_draw_rect(driver_t *drv, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool fill, bool on);
void st7920_draw_bitmap(driver_t *drv, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

/* 지연 함수 */
void st7920_delay_us(uint32_t us);
void st7920_delay_ms(uint32_t ms);

#endif