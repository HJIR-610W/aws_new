#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"

#include "util_time.h"
#include "config_app.h"
#include "bsp_rtc.h"
#include "console_scanf.h"

const char* g_chgList[] = {"화진 스마트", "LS1024"};

int32_t print_menu_system(void)
{
  char buff[50];
  int cnt = 0;

  make_timeToStr(&Date_Time, buff, sizeof(buff));
  io_printf("%2d.시간       :%s\r\n", cnt++, buff);
  io_printf("%2d.아이디(ID) :%d\r\n", cnt++, get_config_app()->id);
  io_printf("%2d.비밀번호   :%d\r\n", cnt++, get_config_app()->password);
  io_printf("%2d:충전기     :%s\r\n", cnt++,
              ITEM_LIST(get_config_app()->charger_model, g_chgList));

  return cnt;
}

int32_t input_date( DATE_TIME_BUF* nt)
{
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
  int cnt;

  io_printf("format:YYYY-MM-DD hh:mm:ss,2020-01-01 00:11:22\r\n");

  cnt = console_scanf("%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec);

  if (cnt == 6)
  {
    nt->Year = year;
    nt->Month = month;
    nt->Day = day;
    nt->Hour = hour;
    nt->Min = min;
    nt->Sec = sec;

    return 6;
  }

  return cnt;
}

int32_t menu_system(void)
{
  int cnt;
  int32_t dec;
  DATE_TIME_BUF nt;

  do
  {
    cnt = select_indexFromList( NULL, print_menu_system, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    switch (cnt)
    {
      case 0:
        cnt = input_date( &nt);
        if (cnt > 0)
        {
          bsp_rtc_set(&nt);
          bsp_rtc_update();
        }
        break;
      case 1:  // id
        cnt = input_decimal( 0, 9999, &dec);
        if (cnt)
        {
          config.id = dec;
          WRITE_CFG(id);
        }
        break;
      case 2:  // password
        cnt = input_decimal( 0, 9999, &dec);
        if (cnt)
        {
          config.password = dec;
          WRITE_CFG(password);
        }
        break;
      case 3:  // charger type
        cnt = select_indexFromList( g_chgList, NULL, sizeof(g_chgList) / sizeof(g_chgList[0]),
                                   true);
        if (cnt > 0)
        {
          cnt--;
          config.charger_model = (eCHARGER_MODEL_t)cnt;
          WRITE_CFG(charger_model);
          io_printf("리셋 후 적용됩니다\r\n");
        }
        break;
    }
  } while (1);
}


#define SYSTEM_MENU_WITDH 30

int aws_menu_system(void)
{
  int choice, status;
  int max_number;
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
             ITEM_LIST(get_config_app()->charger_model, g_chgList));
    menu_cnt++;

    status = choice_menu(SYSTEM_MENU_WITDH, "시스템", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        if (input_date(&nt) > 0)
        {
          bsp_rtc_set(&nt);
          bsp_rtc_update();
        }
        break;
      case 2:  // id
        if (input_decimal(0, 9999, &dec))
        {
          config.id = dec;
          WRITE_CFG(id);
        }
        break;
      case 3:  // password
        if (input_decimal(0, 9999, &dec))
        {
          config.password = dec;
          WRITE_CFG(password);
        }
        break;
      case 4:  // charger type
        status = choice_menu(24,"충전기 종류",(char **)g_chgList,_countof(g_chgList),&choice);
        if(status != MENU_OK)
        {
          return status;;
        }
          config.charger_model = (eCHARGER_MODEL_t)(choice-1);
          WRITE_CFG(charger_model);
          io_printf("리셋 후 적용됩니다\r\n");
        break;
    }

    if (status != MENU_OK)
    {
      break;
    }
  }

  return status;
}