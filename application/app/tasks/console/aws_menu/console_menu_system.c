#include "console_define.h"
#include "console_utile.h"
#include "const_string.h"
#include "debug_io.h"
#include "util_memory.h"

#include "util_time.h"
#include "config_app.h"
#include "drv_rtc.h"
 
 
#include "task_logging.h"




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
    debug_printf("format:YYYY-MM-DD hh:mm:ss,2020-01-01 00:11:22\r\n");

    status = debug_scanf_s("%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec);

    if(status == KEY_CODE_CTRL_P)
    {
      status = MENU_ABORT;
      break;
    }
    else if (status == KEY_CODE_CTRL_C)
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
    debug_printf("입력을 확인해주세요");
  }
  return status;
}


#define SYSTEM_MENU_WITDH 30

int aws_setup_menu_system(void)
{
  const char* menu[4];
  int choice, status;
  char buff[4][40];
  char buffer[25];
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
    make_time_to_string(&Date_Time, buffer, sizeof(buffer));
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "시간    :%s",buffer);
    menu_cnt++;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "ID      :%d", get_config_app()->device_id);
    menu_cnt++;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "비밀번호:%d",get_config_app()->password);
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "충전기  :%s",ITEM_LIST(get_config_app()->charger_model, charger_list_kor));
    menu_cnt++;

    status = view_input_combobox( "시스템", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = input_date(&nt);
          if(status != MENU_OK)
            break;

          drv_rtc_set(&nt);
          drv_rtc_read(&Date_Time);//즉시 표시 

          log_printf(L_INFO, "ST:%d%d%d%d%d%d",nt.Year,nt.Month,nt.Day,nt.Hour,nt.Min,nt.Sec);
        break;
      case 2:  // id
        status = view_input_decimal("ID",&dec,0, 9999);
        if(status != MENU_OK)
          break;
          config.device_id = dec;
          WRITE_CFG(device_id);

        break;
      case 3:  // password
        status = view_input_decimal("비밀번호",&dec,0, 9999);
        if(status != MENU_OK)
          break;
          config.password = dec;
          WRITE_CFG(password);
        break;
      case 4:  // charger type
        status = view_input_combobox( "충전기 종류", charger_list_kor, _countof(charger_list_kor), &choice);
        if(status != MENU_OK)
          break;
          config.charger_model = (eCHARGER_MODEL_t)(choice-1);
          WRITE_CFG(charger_model);
          debug_printf_color(IO_COLOR_RED,"리셋 후 적용됩니다\r\n");
          break;
    }

    if (status == MENU_ABORT)
    {
      break;
    }
  }

  return status;
}