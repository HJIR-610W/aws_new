

#include "menu_system.h"

#include "app_screen.h"
#include "util_time.h"
#include "cli_key_code.h"
#include "view_driver.h"

#include "app_button.h"
#include "config_app.h"
#include "menu_handler.h"

#define SCREEN_COLS 20
#define SYSTEM_WD 10

#define MENU_PRINTF screen_menu_printf_row

//DATE:2025-11-11
//TIME:00:00:00
void draw_menu_system_page(screen_menu_t* p_win)
{
  int row_count = 0;
  char buff[SCREEN_COLS + 1];

  p_win->current_row = 0;

  MENU_PRINTF(p_win, row_count++, "%-*s:%04d-%02d-%02d", SYSTEM_WD, "DATE", Date_Time.Year,
              Date_Time.Month, Date_Time.Day);

  MENU_PRINTF(p_win, row_count++, "%-*s:%02d:%02d:%02d", SYSTEM_WD,"TIME", Date_Time.Hour, Date_Time.Min,
              Date_Time.Sec);

  MENU_PRINTF(p_win, row_count++, "%-*s:%d", SYSTEM_WD, "ID", get_config_app()->id);
  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}




int32_t menu_system(void)
{
  int32_t status;
  int32_t key;
  int32_t page_count = 0;
  int32_t page_list[1];
  screen_menu_t menu;
  uint32_t start_time;

  screen_menu_create(&menu, 8, 20);

  while(1)
  {
    draw_menu_system_page(&menu);
    screen_refresh();

    key = get_button_key(1000);

    if(key == KEY_CODE_ENTER)
    {
      switch (menu.selected_index)
      {
      case 2:
      {
        int val = get_config_app()->id;

        status = input_decimal("ID", 0, 255, &val, SIGN_DISABLE);
        if(status !=MENU_OK)
          break;
        config.id = val;


      }
        break;
      
      default:
        break;
      }
    }
    else if (key != -1)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return status;
}
