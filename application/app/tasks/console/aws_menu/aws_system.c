#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"

#include "util_time.h"
#include "config_app.h"
#include "drv_rtc.h"
#include "console_scanf.h"
#include "cli_input.h"
#include "task_logging.h"

const char* g_chargerList[] = {"화진 스마트", "LS1024"};


int32_t input_date( DATE_TIME_BUF* nt)
{
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
  int status;

  while (1)
  {
    io_printf("format:YYYY-MM-DD hh:mm:ss,2020-01-01 00:11:22\r\n");

    status = cli_scanf_s("%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec);

    if(status == CLI_KEYCODE_CTRL_Q)
    {
      status = MENU_ABORT;
      break;
    }
    else if (status == CLI_KEYCODE_CTRL_C)
    {
      status = MENU_BACK;
      break;
    }

    if(status == 6)
    {
      nt->Year = year;
      nt->Month = month;
      nt->Day = day;
      nt->Hour = hour;
      nt->Min = min;
      nt->Sec = sec;

      status = MENU_OK;
      break;
    }
    io_printf("입력을 확인해주세요");
  }
  return status;
}


#define SYSTEM_MENU_WITDH 30

int aws_setup_menu_system(void)
{
  int choice, status;

  char buff[4][40];
  char buffer[25];
  char* menu[4];
  int menu_cnt = 0;

  DATE_TIME_BUF nt;
  int dec;
  for (int i = 0; i < 4; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;
    make_timeToStr(&Date_Time, buffer, sizeof(buffer));
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "시간    :%s",buffer);
    menu_cnt++;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "ID      :%d", get_config_app()->id);
    menu_cnt++;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "비밀번호:%d",get_config_app()->password);
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "충전기  :%s",
             ITEM_LIST(get_config_app()->charger_model, g_chargerList));
    menu_cnt++;

    status = choice_menu(SYSTEM_MENU_WITDH, "시스템", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = input_date(&nt);
          if(status != MENU_OK)
            break;

          drv_rtc_set(&nt);

          drv_rtc_read(&Date_Time);
          log_printf(L_INFO, "ST:%d%d%d%d%d%d",nt.Year,nt.Month,nt.Day,nt.Hour,nt.Min,nt.Sec);
        break;
      case 2:  // id
        status = input_decimal_prompt("ID",&dec,0, 9999);
        if(status != MENU_OK)
          break;
          config.id = dec;
          WRITE_CFG(id);

        break;
      case 3:  // password
        status = input_decimal_prompt("비밀번호",&dec,0, 9999);
        if(status != MENU_OK)
          break;
          config.password = dec;
          WRITE_CFG(password);
        break;
      case 4:  // charger type
        status = choice_menu(24,"충전기 종류",(char **)g_chargerList,_countof(g_chargerList),&choice);
        if(status != MENU_OK)
          break;
          config.charger_model = (eCHARGER_MODEL_t)(choice-1);
          WRITE_CFG(charger_model);
          io_printf_color(IO_COLOR_RED,"리셋 후 적용됩니다\r\n");
          break;
    }

    if (status == MENU_ABORT)
    {
      break;
    }
  }

  return status;
}