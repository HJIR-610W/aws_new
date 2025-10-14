#include "menu_offset.h"

#include <string.h>
#include "app_logging.h"
#include "app_key.h"
#include "app_screen.h"
#include "app_sensor.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "config_sensor.h"
#include "console_utile.h"
#include "drv_adc.h"
#include "menu_handler.h"
#include "util_memory.h"
#include "view_driver.h"
#include "util_time.h"

#define LOG_VIEW_ROW 7
int32_t menu_view_log(void)
{
  char buffer[22];
  int32_t key;
  int32_t status;
  screen_page_t lcd_win;
  sysLog_t log;
  int update = 1;
  int dec;
  int len;
  int start=0;
  int row=0;
  screen_clear();
  screen_page_create(&lcd_win);
  lcd_win.total_pages = 1;
  lcd_win.chunk_scroll_enable = 1;



  dec = logging_get_logCnt();
  status = input_decimal("Log count", 0, LOG_COUNT_MAX, &dec);
  if(status !=MENU_OK)
    return status;

  do
  {

    if (update)
    {
      screen_clear();
      update = 0;
      // 2025-01-01 01:01:03,INFO,Boot: SW reset
      logging_read_log(dec, &log);

      if(strlen(log.msg)==0)
      {
        screen_printf(0, 0, "Log Count:%d",dec);
        screen_printf(1, 0, "No saved log data");
        screen_refresh();
        goto END_LOOP;
      }
      log.msg[19] = 0;
      log.msg[24] = 0;
      screen_printf(0, 0, "Log Count:%d", dec);
      screen_printf(1, 0, &log.msg[0]);
      screen_printf(2, 0, &log.msg[20]);
      row = 3;
      len = strlen(&log.msg[25]);

      start=0;
      while (start < len)
      {
        if (len - start <= 21)
        {
          screen_printf(row, 0, "%s", &log.msg[25 + start]);
          break;
        }
        else
        {
          memcpy(buffer, &log.msg[25 + start], 21);
          buffer[21] = 0;
          screen_printf(row, 0, "%s", buffer);
          start += 21;
          row++;
        }
        }

        screen_refresh();
    }

END_LOOP:
        key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }
    else if (key == KEY_CODE_UP)
    {
        dec--;
        update = 1;
    }
    else if (key == KEY_CODE_DOWN)
    {
        dec++;
        update = 1;
    }
    } while (1);

    return convert_key_to_status(key);

}

#define DEV_LOG 0


void draw_menu_developer_page(screen_menu_t *p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, DEV_LOG, "System log");
  screen_menu_clear(p_win);
}

int32_t setup_menu_developer(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, "Developer");

  while (1)
  {
    draw_menu_developer_page(&menu);
    screen_refresh();

    key = get_menu_key(1000);

    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
    else if (key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
      case DEV_LOG:
      status = menu_view_log();
      break;
      default:
        break;
      }
      if (status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}