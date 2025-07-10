#include "menu_panel.h"

#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "console_utile.h"
#include "menu_handler.h"
#include "util_memory.h"
#include "view_driver.h"

#define SCREEN_COLS 20
#define PANEL_WD 8

#define MENU_PRINTF screen_menu_printf_row

const char* g_panelList_lcd[] = {"AWS STD", "HJ STD", "MOOJU", "HANSUNG"};

#define PANEL_MENU_MODEL    0
#define PANEL_MENU_SNOW     1
#define PANEL_MENU_BAROMETER 2

void draw_setup_menu_panel_page(screen_menu_t* p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, PANEL_MENU_MODEL);
  MENU_PRINTF(p_win, row_count++, "%-*s:%s", PANEL_WD, "MODEL",
              ITEM_LIST(get_config_app()->panel_model, g_panelList_lcd));

  if (get_config_app()->panel_model == ePANEL_MUJU)
  {
    screen_update_list(p_win, row_count, PANEL_MENU_SNOW);
    MENU_PRINTF(p_win, row_count++, "%-*s:%s", PANEL_WD, "SNOW",
                ITEM_LIST((int32_t)get_config_app()->panel_snow_use, enableList));

    screen_update_list(p_win, row_count, PANEL_MENU_BAROMETER);
    MENU_PRINTF(p_win, row_count++, "%-*s:%s", PANEL_WD, "BAROM",
                ITEM_LIST((int32_t)get_config_app()->panel_barometer_use, enableList));
  }

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

int32_t setup_menu_panel(void)
{
  int32_t choice = 0;
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, 8, 20);

  while (1)
  {
    draw_setup_menu_panel_page(&menu);
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
        case PANEL_MENU_MODEL:
        {
          status = print_menu_list(g_panelList_lcd, _countof(g_panelList_lcd), &choice);
          if (status != MENU_OK)
            break;
          config.panel_model = (ePANEL_MODEL_t)choice;
          WRITE_CFG(panel_model);
        }
        break;

        case PANEL_MENU_SNOW:
        {
          bool enable = get_config_app()->panel_snow_use;
          status = print_menu_list(enableList, _countof(enableList), &choice);
          if (status != MENU_OK)
            break;
          config.panel_snow_use = (uint8_t)choice;
          WRITE_CFG(panel_snow_use);
        }
        break;

        case PANEL_MENU_BAROMETER:
        {
          bool enable = get_config_app()->panel_barometer_use;
          status = print_menu_list(enableList, _countof(enableList), &choice);
          if (status != MENU_OK)
            break;
          config.panel_barometer_use = (uint8_t)choice;
          WRITE_CFG(panel_barometer_use);
        }
        break;

        default:
          break;
      }
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}