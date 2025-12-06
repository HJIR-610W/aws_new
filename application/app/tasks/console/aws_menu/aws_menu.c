
#include "aws_menu.h"

#include "aws_develop.h"
#include "aws_menu_cali.h"
#include "aws_menu_data.h"

#include "aws_menu_offset.h"
#include "aws_menu_panel.h"
#include "aws_menu_sensor.h"
#include "aws_network.h"
#include "aws_system.h"
#include "console_define.h"
#include "console_utile.h"
#include "debug_io.h"
#include "util_memory.h"
#include "aws_menu_view.h"
#include "aws_menu_manager.h"

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


#define MENU_ITEM(def) GET_1 def
#define MENU_NUMBER(def) GET_2 def

int aws_menu(void)
{
  int choice, status;

 const char* menu[] = { MENU_ITEM(MENU_INFO_DEF),
                        MENU_ITEM(MENU_SYSTEM_DEF),
                        MENU_ITEM(MENU_SENSOR_DEF),
                        MENU_ITEM(MENU_NETWORK_DEF),
                        MENU_ITEM(MENU_DATA_DEF),
                        MENU_ITEM(MENU_PANEL_DEF),
                        MENU_ITEM(MENU_OFFSET_DEF),
                        MENU_ITEM(MENU_CALIBRATION_DEF),
                        MENU_ITEM(MENU_MANAGER_DEF)};

  while (1)
  {
    status = view_input_combobox("AWS", menu, _countof(menu), &choice);

    switch (choice)
    {
      case MENU_NUMBER(MENU_INFO_DEF):
        aws_menu_veiw();
        break;
      case MENU_NUMBER(MENU_SYSTEM_DEF):
        aws_setup_menu_system();
        break;
      case MENU_NUMBER(MENU_SENSOR_DEF):
        aws_menu_sensor();
        break;
      case MENU_NUMBER(MENU_NETWORK_DEF):
        aws_menu_network();
        break;
      case MENU_NUMBER(MENU_DATA_DEF):
        aws_menu_data();
        break;
      case MENU_NUMBER(MENU_PANEL_DEF):
        aws_menu_panel();
        break;
      case MENU_NUMBER(MENU_OFFSET_DEF):
        aws_menu_offset();
        break;
      case MENU_NUMBER(MENU_CALIBRATION_DEF):
        aws_menu_calibration();
        break;
      case MENU_NUMBER(MENU_MANAGER_DEF):
        aws_menu_manager();
        break; ;
    }
  }

  return 0;
}