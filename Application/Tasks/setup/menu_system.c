

#include "menu_system.h"

#include "app_screen.h"
#include "util_time.h"
#include "cli_key_code.h"
#include "view_driver.h"

#include "app_button.h"
#define SCREEN_COLS 20
#define SYSTEM_WD 10

//DATE:2025-11-11
//TIME:00:00:00
void draw_menu_system_page(screen_page_t* p_win)
{
  int row_count = 0;
  char buff[SCREEN_COLS + 1];

  p_win->current_row = 0;


  screen_printf_row(p_win, row_count++, "SYSTEM");

  screen_printf_row(p_win, row_count++, "DATE:%04d-%02d-%02d", Date_Time.Year,
                    Date_Time.Month, Date_Time.Day);

  screen_printf_row(p_win, row_count++, "TIME:%02d:%02d:%02d", Date_Time.Hour, Date_Time.Min, Date_Time.Sec);


  p_win->total_items[0] = ALIGN_UP(row_count, p_win->view_row);

  while (p_win->current_row < p_win->view_row)
  {
    screen_clear_row(p_win, row_count++);
  }
}


int32_t menu_system(void)
{
  int32_t status;
  int32_t key;
  int32_t page_count = 0;
  int32_t page_list[1];
  screen_page_t lcd_win;
  uint32_t start_time;

  screen_page_create(&lcd_win, 8, 20);

  lcd_win.chunk_scroll_use = 0;
  lcd_win.multi_page_use = 0;

  while(1)
  {
    draw_menu_system_page(&lcd_win);
    screen_refresh();

    key = get_button_key(1000);

    if(key == KEY_CODE_ENTER)
    {

    }
    else if (key != -1)
    {
      screen_handle_scroll(&lcd_win, key);
    }
  }

  return status;
}
