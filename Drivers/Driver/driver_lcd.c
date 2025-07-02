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
    default:
      break;
  }

  return driver;
}

void driver_lcd_set_position(driver_t *drv, uint8_t x, uint8_t y)
{
  if(drv==NULL)
  {
    return;
  }

  lcd_api_t *api = (lcd_api_t *)drv->api;

  api->set_position(drv,x,y);

}

void driver_lcd_write_string(driver_t *drv, const char *str)
{
  if (drv == NULL)
  {
    return;
  }

  lcd_api_t *api = (lcd_api_t *)drv->api;

  api->write_string(drv, str);
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