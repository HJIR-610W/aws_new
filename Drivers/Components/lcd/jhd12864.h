#ifndef JHD12864_H
#define JHD12864_H

#include <stdint.h>
#include <stdbool.h>

#include "driver_interface.h"

/* CS 신호 정의 */
typedef enum {
    JHD12864_CS_LEFT = 0,   // CS1 - 좌측 반쪽 (X: 0-63)
    JHD12864_CS_RIGHT = 1,  // CS2 - 우측 반쪽 (X: 64-127)
    JHD12864_CS_BOTH = 2    // 양쪽 모두
} jhd12864_cs_t;

driver_t* jhd12864_open(void);
void jhd12864_reset(driver_t *drv);
void jhd12864_send_cmd(driver_t *drv, uint8_t cmd);
void jhd12864_send_data(driver_t *drv, uint8_t data);

void jhd12864_clear_screen(driver_t *drv);
void jhd12864_home(driver_t *drv);
void jhd12864_display_on(driver_t *drv);
void jhd12864_display_off(driver_t *drv);

/* 모드 및 텍스트 */
void jhd12864_set_graphic_mode(driver_t *drv, bool enable);
void jhd12864_set_position(driver_t *drv, uint8_t row, uint8_t col);
void jhd12864_write_string(driver_t *drv, int row, int col, const char *str);
void jhd12864_write_string_simple(driver_t *drv, const char *str);

/* JHD12864 특화 함수 */
void jhd12864_set_page(driver_t *drv, uint8_t page);
void jhd12864_set_column(driver_t *drv, uint8_t column);
void jhd12864_flush_buffer(driver_t *drv);

/* CS 제어 함수 */
void jhd12864_select_cs(driver_t *drv, jhd12864_cs_t cs);
jhd12864_cs_t jhd12864_get_cs_from_x(uint8_t x);

/* 그래픽 출력 - 그래픽 모드에서만 동작 */
void jhd12864_set_pixel(driver_t *drv, uint8_t x, uint8_t y, bool on);
void jhd12864_draw_line(driver_t *drv, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool on);
void jhd12864_draw_rect(driver_t *drv, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool fill, bool on);
void jhd12864_draw_bitmap(driver_t *drv, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

/* 지연 함수 */
void jhd12864_delay_us(uint32_t us);
void jhd12864_delay_ms(uint32_t ms);

/* GPIO 제어 함수 */
void jhd12864_gpio_init(void);
void jhd12864_gpio_set_data_bus(uint8_t data);
void jhd12864_gpio_write_byte(uint8_t data, bool is_cmd);
void jhd12864_gpio_set_data_bus_input(void);
void jhd12864_gpio_set_data_bus_output(void);
uint8_t jhd12864_gpio_read_data_bus(void);
uint8_t jhd12864_gpio_read_byte(bool is_cmd);

/* 레지스터 읽기 함수 */
uint8_t jhd12864_read_status(driver_t *drv);
uint8_t jhd12864_read_data(driver_t *drv);
bool jhd12864_is_busy(driver_t *drv);
void jhd12864_wait_ready(driver_t *drv);

#endif