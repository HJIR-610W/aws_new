
#include "menu_handler.h"
#include "menu_network.h"
#include "menu_sensor.h"
#include "menu_system.h"
#include "util_memory.h"
#include "menu_offset.h"
#include "menu_panel.h"
#include "menu_calibration.h"
#include "menu_manager.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include  "app_key.h"
#include "menu_data.h"

#define AWS_SETUP_SYSTEM 0
#define AWS_SETUP_SENSOR 1
#define AWS_SETUP_NETWORK 2
#define AWS_SETUP_DATA 3
#define AWS_SETUP_PANEL  4
#define AWS_SETUP_OFFSET 5
#define AWS_SETUP_CALI 6
#define AWS_SETUP_MANAGER 7

#define SETUP_WD 15

#define MENU_PRINTF screen_menu_printf_row

void draw_aws_setup_page(screen_menu_t* p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, AWS_SETUP_SYSTEM);
  MENU_PRINTF(p_win, row_count++, "%-*s", SETUP_WD, "System");
  screen_update_list(p_win, row_count, AWS_SETUP_SENSOR);
  MENU_PRINTF(p_win, row_count++, "%-*s", SETUP_WD, "Sensor");
  screen_update_list(p_win, row_count, AWS_SETUP_NETWORK);
  MENU_PRINTF(p_win, row_count++, "%-*s", SETUP_WD, "Network");
  screen_update_list(p_win, row_count, AWS_SETUP_DATA);
  MENU_PRINTF(p_win, row_count++, "%-*s", SETUP_WD, "Data");
  screen_update_list(p_win, row_count, AWS_SETUP_PANEL);
  MENU_PRINTF(p_win, row_count++, "%-*s", SETUP_WD, "Panel");
  screen_update_list(p_win, row_count, AWS_SETUP_OFFSET);
  MENU_PRINTF(p_win, row_count++, "%-*s", SETUP_WD, "Offset");
  screen_update_list(p_win, row_count, AWS_SETUP_CALI);
  MENU_PRINTF(p_win, row_count++, "%-*s", SETUP_WD, "Calibraion");
  screen_update_list(p_win, row_count, AWS_SETUP_MANAGER);
  MENU_PRINTF(p_win, row_count++, "%-*s", SETUP_WD, "Manager");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}
void setup_root(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, "AWS Setup");

  while(1)
  {
    draw_aws_setup_page(&menu);
    screen_refresh();

    key = get_button_key(1000);

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
        case AWS_SETUP_SYSTEM:  // SYSTEM
          status = setup_menu_system();
          break;
        case AWS_SETUP_SENSOR:
          status = setup_menu_sensor();
          break;
        case AWS_SETUP_NETWORK:
          status = setup_menu_network();
          break;
        case AWS_SETUP_DATA:  // SYSTEM
          status  = setup_menu_data();
          break;
        case AWS_SETUP_PANEL:
          status = setup_menu_panel();
          break;
        case AWS_SETUP_OFFSET:
          status = setup_menu_offset();
          break;
        case AWS_SETUP_CALI:
          status = setup_menu_calibration();
          break;
        case AWS_SETUP_MANAGER:
          status = setup_menu_manager();
          break;
        default:
          break;

      }

      if (status == MENU_ABORT)
        return ;
    }
    else if (key != -1)
    {
      screen_menu_handle(&menu, key);
    }
  }


}