




#ifndef DRIVER_LCD_H
#define DRIVER_LCD_H

#include "driver_interface.h"
#include "drv_lcd_define.h"

#define DRIVER_CLCD 0
#define DRIVER_LCD_TERMNINAL 1


driver_t *driver_lcd_open(int num);
void drv_lcd_close(int num);

void driver_lcd_set_position(driver_t *drv, uint8_t row, uint8_t col);
void driver_lcd_write_string(driver_t *drv, const char *str);

void driver_lcd_clear_screen(driver_t *drv);
void driver_lcd_home(driver_t *drv);
void driver_lcd_display_on(driver_t *drv);
void driver_lcd_display_off(driver_t *drv);
void driver_lcd_set_mode(driver_t *drv, eLCD_MODE_t lcd_mode);
void driver_lcd_set_pixel(driver_t *drv, uint8_t x, uint8_t y, bool on);
void driver_lcd_draw_line(driver_t *drv, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool on);
void driver_lcd_flush(driver_t *drv);
void driver_lcd_put_ch(driver_t *drv, int row, int col, uint8_t ch);
void driver_write_string_at(driver_t *drv, int row, int col, const char *str);

void driver_lcd_backup_framebuffer(int num);
void driver_lcd_restore_framebuffer(int num);
#endif