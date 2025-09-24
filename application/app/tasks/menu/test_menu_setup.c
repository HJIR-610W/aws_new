
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

void test_draw_aws_setup_page(screen_menu_t* p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, AWS_SETUP_CALI,   "Calibraion");

  screen_menu_clear(p_win);
}






void test_setup_menu(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  int32_t admin_menu_active_count=0;
  screen_menu_t menu;
  int32_t password=0;


  screen_menu_create(&menu, "Test");

  while(1)
  {
    test_draw_aws_setup_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case AWS_SETUP_CALI:
          status = setup_menu_calibration();
          break;

        default:
          break;

      }
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_menu_handle(&menu, key);
    }
  }

}