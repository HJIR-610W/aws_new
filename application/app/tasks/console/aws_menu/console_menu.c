
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
#include "system_err.h"



void menu_mirror(void)
{
  debug_printf(ES_CURSOR_HIDE);
  debug_printf(ES_CLEAR_SCREEN);
  debug_printf(ES_CURSOR_POSITION(10,1));//실제 화면은 1~8 이고 9행은 정보
  debug_printf("CTRL+Z 종료,ESC CTRL+C,LONG ESC CTRL+Q,LONG ENTER CTRL+P\r\n");

  g_log_write_enable  = 0;
  g_debug_port_mirror_enable = 1;

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

void menu_mirror_with_log(void)
{
  debug_printf(ES_CURSOR_HIDE);
  debug_printf(ES_CLEAR_SCREEN);
  debug_printf(ES_CURSOR_POSITION(10,1));//실제 화면은 1~8 이고 9행은 정보
  debug_printf("CTRL+Z 종료,ESC CTRL+C,LONG ESC CTRL+Q,LONG ENTER CTRL+P\r\n");
  debug_printf(EC_SCROLL_REGION(12,40 ));
  debug_printf(ES_CURSOR_POSITION(12,1));
  //g_log_write_enable  = 0;
  g_debug_port_mirror_enable = 1;

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
  //g_log_write_enable  = 1;
  debug_printf(ES_CURSOR_POSITION(1,1));//실제 화면은 1~8 이고 9행은 정보
  debug_printf(ES_CLEAR_SCREEN_BELOW);
  
  debug_printf(ES_CURSOR_SHOW);
  debug_printf(EC_SCROLL_REGION_RESET);

}

extern void testColsoleTask(void *arg);
int aws_menu(void)
{
  int choice =0, status;
  int test_mode_num=0;

 const char* menu[] = {"기본 정보",
                       "LCD 화면(로그 포함)",
                       "LCD 화면"};

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
        menu_mirror_with_log();    
      break;
      case 3:
      menu_mirror();
      break;

    }
  }

  //return 0;
}