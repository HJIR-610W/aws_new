
#include "console_menu.h"

#include "aws_develop.h"
#include "console_menu_cali.h"

#include "console_menu_offset.h"
#include "console_menu_panel.h"
#include "console_menu_network.h"
#include "console_menu_system.h"
#include "console_menu_panel.h"
#include "console_menu_data.h"
#include "console_menu_view.h"
#include "console_menu_manager.h"
#include "console_menu_sensor.h"

#include "console_login.h"
#include "console_define.h"
#include "console_utile.h"
#include "debug_io.h"
#include "util_memory.h"
#include "app_screen.h"
#include "app_key.h"


#define MENU_INFO        1
#define MENU_SYSTEM      2
#define MENU_SENSOR      3
#define MENU_NETWORK     4
#define MENU_DATA        5
#define MENU_PANEL       6
#define MENU_OFFSET      7
#define MENU_CALIBRATION 8
#define MENU_MANAGER     9 

#define MENU_INFO_DEF         ( "기본정보", MENU_INFO)
#define MENU_SYSTEM_DEF       ( "시스템", MENU_SYSTEM)
#define MENU_SENSOR_DEF       ( "센서", MENU_SENSOR)
#define MENU_NETWORK_DEF      ( "네트워크", MENU_NETWORK)
#define MENU_DATA_DEF         ( "데이터", MENU_DATA)
#define MENU_PANEL_DEF        ( "패널(전광판)", MENU_PANEL)
#define MENU_OFFSET_DEF       ( "오프셋", MENU_OFFSET)
#define MENU_CALIBRATION_DEF  ( "켈리브레이션", MENU_CALIBRATION)
#define MENU_MANAGER_DEF      ( "관리", MENU_MANAGER)

#define MENU_SYSTEM_DEF       ( "시스템", MENU_SYSTEM)

#define MENU_ITEM(def) GET_1 def
#define MENU_NUMBER(def) GET_2 def


void menu_mirror(void)
{
  g_log_write_enable  = 0;
  g_debug_port_mirror_enable = 1;

  debug_printf(ES_CURSOR_HIDE);
  debug_printf(ES_CLEAR_SCREEN);

  uint8_t ch;
  while(1)
  {
    debug_get_ch(&ch);
    inject_key(ch);
    if(ch==KEY_CODE_CTRL_Z) 
    {
      break;
    }
  }
  g_debug_port_mirror_enable = 0;
  g_log_write_enable  = 1;
  debug_printf(ES_CURSOR_SHOW);

}



extern void testColsoleTask(void *arg);
int aws_menu(void)
{
  int choice =0, status;
  int test_mode_num=0;

 const char* menu[] = {"기본 정보",
                       "화면 미러"};

  while (1)
  {
    status = view_input_combobox("AWS", menu, _countof(menu), &choice);

    if(status!=MENU_OK)
    {

      test_mode_num++;
      if(test_mode_num==5)
      {
        test_mode_num=0;
        if(check_login("1601"))
        {
          testColsoleTask(0);
        }
      }
      continue;
    }
    test_mode_num = 0;
    switch (choice)
    {
      case 1:
        aws_menu_veiw();
        break;
      case 2:
        menu_mirror();    
      break;

    }
  }

  //return 0;
}