#include "menu_panel.h"

#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "console_utile.h"
#include "menu_handler.h"
#include "util_memory.h"
#include "view_driver.h"
#include "common\const_string.h"

#define SCREEN_COLS 20
#define PANEL_WD 8



#define PANEL_MENU_MODEL    0
#define PANEL_MENU_SNOW     1
#define PANEL_MENU_BAROMETER 2
#define PANEL_MENU_ITEM6    3

void draw_setup_menu_panel_page(screen_menu_t* p_win)
{

  screen_menu_start(p_win);

  screen_menu_printf(p_win, PANEL_MENU_MODEL, "%-*s:%s", PANEL_WD, "Moel",ITEM_LIST(get_config_app()->panel_model, panel_list_eng));

  if (get_config_app()->panel_model == ePANEL_MUJU)
  {
    screen_menu_printf(p_win, PANEL_MENU_SNOW, "%-*s:%s", PANEL_WD, "Snow",
                       ITEM_LIST((int32_t)get_config_app()->panel_snow_active, enable_list_eng));
    screen_menu_printf(p_win, PANEL_MENU_BAROMETER, "%-*s:%s", PANEL_WD, "Baro",
                       ITEM_LIST((int32_t)get_config_app()->panel_barometer_active, enable_list_eng));
  }
  else   if (get_config_app()->panel_model == ePANEL_ITEM6)
  {
        screen_menu_printf(p_win, PANEL_MENU_ITEM6, "%-*s:%s", PANEL_WD, "TYPE",        ITEM_LIST((int32_t)get_config_app()->panel_item6_type, panel_item6_type_list_eng));
  }

  screen_menu_clear(p_win);

}

int32_t setup_menu_panel(void)
{
  int32_t choice = 0;
  int32_t index;
  int32_t key;
  int32_t status = MENU_OK;
  screen_menu_t menu;

  screen_menu_create(&menu,  "Panel");

  while (1)
  {
    draw_setup_menu_panel_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
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
          int pre_panel_model;
          choice = config.panel_model;
          pre_panel_model = choice;
          status = input_combobox("Panel Model",panel_list_eng, _countof(panel_list_eng), &choice);
          if (status != MENU_OK)
            break;
          if(config.panel_model != choice)
          {
            config.panel_model = (ePANEL_MODEL_t)choice;
            WRITE_CFG(panel_model);
            if (pre_panel_model == ePANEL_NOT_USED || choice == ePANEL_NOT_USED) //미사용에서 사용으로, 사용에서 미사용은 리셋후 적용됨
            {
              show_popup("Information", "Applied after reset");
            }
          }
         
        }
        break;

        case PANEL_MENU_SNOW:
        {
           choice = get_config_app()->panel_snow_active;

           status = input_active("Snow Active", &choice);
           if (status != MENU_OK)
             break;
           config.panel_snow_active = (uint8_t)choice;
           WRITE_CFG(panel_snow_active);
        }
        break;

        case PANEL_MENU_BAROMETER:
        {
           choice = get_config_app()->panel_barometer_active;
           status = input_active("Barometer Active", &choice);
           if (status != MENU_OK)
             break;
           config.panel_barometer_active = (uint8_t)choice;
           WRITE_CFG(panel_barometer_active);
        }
        break;
      case PANEL_MENU_ITEM6:
        {

          choice = config.panel_item6_type;
          status = input_combobox("Type",panel_item6_type_list_eng, _countof(panel_item6_type_list_eng), &choice);
          if (status != MENU_OK)
            break;

            config.panel_item6_type = (ePANEL_ITEM6_TYPE_t)choice;
            WRITE_CFG(panel_item6_type);
      
        }
        break;
        default:
          break;
      }
      if(status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}