#include <stdarg.h>
#include <stdio.h>


#include "driver_lcd.h"
#include "Components\lcd\st7920.h"
#include "driver_lcd_define.h"
#include "Components\lcd\vt100_terminal.h"



driver_t *driver_lcd_open(int num)
{
  driver_t *driver = NULL;

  switch (num)
  {
    case DRIVER_CLCD:
      driver = st7920_open();
      break;
    case DRIVER_LCD_TERMNINAL:
      driver = vt100_terminal_open();
      break;

    default:
      break;
  }

  return driver;
}

void driver_lcd_set_position(driver_t *drv, uint8_t row, uint8_t col)
{
  if(drv==NULL)
  {
    return;
  }

  lcd_api_t *api = (lcd_api_t *)drv->api;

  api->set_position(drv,row,col);

}


void driver_lcd_clear_screen(driver_t *drv)
{
  if (drv == NULL)
    return;
  lcd_api_t *api = (lcd_api_t *)drv->api;

  api->clear_screen(drv);
}
void driver_lcd_home(driver_t *drv)
{
  if (drv == NULL)
    return;
  lcd_api_t *api = (lcd_api_t *)drv->api;

  api->home(drv);
}
void driver_lcd_display_on(driver_t *drv)
{
  if (drv == NULL)
    return;
  lcd_api_t *api = (lcd_api_t *)drv->api;

  api->display_on(drv);
}
void driver_lcd_display_off(driver_t *drv)
{
  if (drv == NULL)
    return;
  lcd_api_t *api = (lcd_api_t *)drv->api;

  api->display_off(drv);
}

void driver_lcd_set_mode(driver_t *drv, eLCD_MODE_t lcd_mode)
{
  if (drv == NULL)
    return;
  lcd_api_t *api = (lcd_api_t *)drv->api;

  if(api->set_mode != NULL)
    api->set_mode(drv, lcd_mode);
}

void driver_lcd_set_pixel(driver_t *drv, uint8_t x, uint8_t y, bool on)
{
  if (drv == NULL)
    return;
  lcd_api_t *api = (lcd_api_t *)drv->api;

  if(api->set_pixel != NULL)
    api->set_pixel(drv, x, y, on);
}

void driver_lcd_draw_line(driver_t *drv, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool on)
{
  if (drv == NULL)
    return;
  lcd_api_t *api = (lcd_api_t *)drv->api;

  if(api->draw_line != NULL)
    api->draw_line(drv, x1, y1, x2, y2, on);
}



void driver_lcd_flush(driver_t *drv)
{
  if (drv == NULL )
    return;

  lcd_api_t *api = (lcd_api_t *)drv->api;

  if(api->flush)
  api->flush(drv);

}

void driver_lcd_put_ch(driver_t *drv, int row, int col, uint8_t ch)
{
  if (drv == NULL)
    return;

  lcd_api_t *api = (lcd_api_t *)drv->api;

  if (api->put_ch)
    api->put_ch(drv,row,col,ch);
}

void driver_write_string_at(driver_t *drv, int row, int col, const char *str)
{
  if (drv == NULL)
    return;

  lcd_api_t *api = (lcd_api_t *)drv->api;

  if (api->write_string_at)
    api->write_string_at(drv, row, col, str);
}