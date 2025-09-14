
#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "menu_handler.h"
#include "menu_network.h"
#include "menu_sensor.h"
#include "menu_system.h"
#include "menu_offset.h"
#include "menu_panel.h"
#include "menu_calibration.h"
#include "menu_manager.h"
#include "menu_data.h"
#include "menu_admin.h"
#include "util_memory.h"


#define AWS_SETUP_SYSTEM    0
#define AWS_SETUP_SENSOR    1
#define AWS_SETUP_NETWORK   2
#define AWS_SETUP_DATA      3
#define AWS_SETUP_PANEL     4
#define AWS_SETUP_OFFSET    5
#define AWS_SETUP_CALI      6
#define AWS_SETUP_MANAGER   7
#define AWS_SETUP_DEVELOPER 8


static bool g_admin_menu_active=false; //필요에 의해서만 developer 메뉴 활성화 목적

void draw_aws_setup_page(screen_menu_t* p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, AWS_SETUP_SYSTEM, "System");
  screen_menu_printf(p_win, AWS_SETUP_SENSOR, "Sensor");
  screen_menu_printf(p_win, AWS_SETUP_NETWORK,"Network");
  screen_menu_printf(p_win, AWS_SETUP_DATA,   "Data");
  screen_menu_printf(p_win, AWS_SETUP_PANEL,  "Panel");
  screen_menu_printf(p_win, AWS_SETUP_OFFSET, "Offset");
  screen_menu_printf(p_win, AWS_SETUP_CALI,   "Calibraion");
  screen_menu_printf(p_win, AWS_SETUP_MANAGER,"Manager");
  if(g_admin_menu_active)
  {
    screen_menu_printf(p_win, AWS_SETUP_DEVELOPER, "Developer");
  }
  screen_menu_clear(p_win);
}

#define MENU_PASSWORD 7777
void setup_menu(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  int32_t admin_menu_active_count=0;
  screen_menu_t menu;
  int32_t password=0;

  while(1)
  {
    status = input_password("PASS WORD",&password);

    if(status != MENU_OK)
    return;

    if (password == MENU_PASSWORD)
    {
      break;
    }
    else
    {
      show_popup("Error", "Incorrect password");
    }
  
  }

  screen_menu_create(&menu, "AWS Setup");

  while(1)
  {
    draw_aws_setup_page(&menu);
    screen_refresh();

    key = get_button_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }
    else if(key== KEY_CODE_RIGHT)
    {
      if (menu.index_list[menu.selected_index] == AWS_SETUP_MANAGER)
      {
         admin_menu_active_count++;
        if (admin_menu_active_count == 5)
        {
          g_admin_menu_active = true;
        }

      }
    }



    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case AWS_SETUP_SYSTEM:
          status = setup_menu_system();
          break;
        case AWS_SETUP_SENSOR:
          status = setup_menu_sensor();
          break;
        case AWS_SETUP_NETWORK:
          status = setup_menu_network();
          break;
        case AWS_SETUP_DATA:
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
        case AWS_SETUP_DEVELOPER:
          status = setup_menu_developer();
          break;
        default:
          break;

      }
      if (status == MENU_ABORT)
        return ;
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_menu_handle(&menu, key);

    }
  }
}