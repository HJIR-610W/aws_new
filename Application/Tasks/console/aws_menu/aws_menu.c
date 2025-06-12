#include "console_define.h"
#include "console_utile.h"
#include "util_memory.h"
#include "dev_io.h"

#include "console_aws_display.h"
#include "aws_system.h"
#include "console_cali.h"
#include "aws_menu_manager.h"
#include "aws_menu_panel.h"
#include "aws_menu_sensor.h"
#include "aws_network.h"
#include "aws_menu_data.h"
#include "aws_develop.h"

#define AWS_MENU_WIDTH 30
int aws_menu(void)
{
  int choice, status;

  char* menu[] = { "기본정보",
                   "시스템",
                   "센서",
                   "네트워크",
                   "데이터",
                   "패널(전광판)",
                   "켈리브레이션",
                   "관리"};

  while (1)
  {
    status = choice_menu(AWS_MENU_WIDTH, "AWS", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      break ;

    switch (choice)
    {
      case 1:
        aws_menu_display();
        break;
      case 2:
        aws_menu_system();
        break;
      case 3:
        aws_menu_sensor();
        break;
      case 4:
        aws_menu_network();
        break;
      case 5:
        aws_menu_data();
        break;
      case 6:
        aws_menu_panel();
         break;
      case 7:
        aws_menu_calibration();
      break;
      case 8:
        aws_menu_manager();
        break; ;

    }
  }

  return 0;
}