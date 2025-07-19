
#ifndef DRIVER_LCD_DEFINE_H
#define DRIVER_LCD_DEFINE_H

#include "driver_interface.h"

typedef enum{
  eLCD_MODE_CHARACTER,
  eLCD_MODE_GRAPHIC
}eLCD_MODE_t;

typedef struct lcd_api_s
{
  void (*set_position)(driver_t *drv, uint8_t row, uint8_t col);
  void (*write_string_at)(driver_t *drv, int row, int col, const char *str);
  void (*clear_screen)(driver_t *drv);
  void (*home)(driver_t *drv);
  void (*display_on)(driver_t *drv);
  void (*display_off)(driver_t *drv);
  void (*set_mode)(driver_t *drv, eLCD_MODE_t lcd_mode);
  void (*set_pixel)(driver_t *drv, uint8_t x, uint8_t y, bool on);
  void (*draw_line)(driver_t *drv, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool on);
  void (*flush)(driver_t *drv);
  void  (*put_ch)(driver_t *drv, int row, int col, uint8_t ch);
} lcd_api_t;
#endif