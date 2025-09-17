

#include "menu_system.h"

#include "app_key.h"
#include "app_screen.h"
#include "drv_rtc.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "const_string.h"
#include "console_utile.h"
#include "menu_handler.h"
#include "util_time.h"
#include "view_driver.h"
#include "task_logging.h"

#define SYSTEM_WD 8



#define SYSTEM_MENU_DATE     0
#define SYSTEM_MENU_TIME     1
#define SYSTEM_MENU_ID       2
#define SYSTEM_MENU_PASSWORD 3
#define SYSTEM_MENU_CHARGER  4


void draw_setup_menu_system_menu(screen_menu_t* p_win)
{
  screen_menu_start(p_win);

  screen_menu_printf(p_win, SYSTEM_MENU_DATE, "%-*s:%04d-%02d-%02d", SYSTEM_WD, "DATE", Date_Time.Year,
                     Date_Time.Month, Date_Time.Day);
  screen_menu_printf(p_win, SYSTEM_MENU_TIME, "%-*s:%02d:%02d:%02d", SYSTEM_WD, "TIME", Date_Time.Hour,
                     Date_Time.Min, Date_Time.Sec);
  screen_menu_printf(p_win, SYSTEM_MENU_ID, "%-*s:%d", SYSTEM_WD, "ID", get_config_app()->id);
  screen_menu_printf(p_win, SYSTEM_MENU_PASSWORD, "%-*s:%d", SYSTEM_WD, "PASS", get_config_app()->password);
  screen_menu_printf(p_win, SYSTEM_MENU_CHARGER, "%-*s:%s", SYSTEM_WD, "CHARGER",
                     ITEM_LIST(get_config_app()->charger_model, g_charger_list_eng));
  screen_menu_clear(p_win);
}

int32_t setup_menu_system(void)
{
  int32_t choice=0;
  int32_t status;
  int32_t key;
  int32_t index;
  screen_menu_t menu;

  screen_menu_create(&menu, "System");

  while(1)
  {
    draw_setup_menu_system_menu(&menu);
    screen_refresh();

    key = get_menu_key(1000);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if(key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case SYSTEM_MENU_DATE:
        {
          string_fmt_t strfmt;
          DATE_TIME_BUF nt;
          strfmt.fmt = "%04d-%02d-%02d";
          snprintf(strfmt.data, sizeof(strfmt.data), "%04d-%02d-%02d",Date_Time.Year,Date_Time.Month,Date_Time.Day);
          status = input_fmt(&strfmt, "Date");
          if (status != MENU_OK)
            break;
          int year;
          int month;
          int day;
          sscanf(strfmt.data, strfmt.fmt,&year,&month,&day);
          nt =Date_Time;
          nt.Year = year;
          nt.Month = month;
          nt.Day = day;
          drv_rtc_set(&nt);

                    log_printf(L_INFO, "ST:%d%d%d%d%d%d",nt.Year,nt.Month,nt.Day,nt.Hour,nt.Min,nt.Sec);
        }
        break;
        case SYSTEM_MENU_TIME:
        {
          string_fmt_t strfmt;
          DATE_TIME_BUF nt;
          strfmt.fmt = "%02d:%02d:%02d";
          snprintf(strfmt.data, sizeof(strfmt.data), strfmt.fmt, Date_Time.Hour, Date_Time.Min,
                   Date_Time.Sec);
          status = input_fmt(&strfmt, "Time");
          if (status != MENU_OK)
            break;
          int hour;
          int min;
          int sec;
          sscanf(strfmt.data, strfmt.fmt, &hour, &min, &sec);
          nt = Date_Time;
          nt.Hour = hour;
          nt.Min = min;
          nt.Sec = sec;
          drv_rtc_set(&nt);
          log_printf(L_INFO, "ST:%d%d%d%d%d%d",nt.Year,nt.Month,nt.Day,nt.Hour,nt.Min,nt.Sec);
        }
      break;
      case SYSTEM_MENU_ID:
      {
        int val = get_config_app()->id;
        status = input_decimal("ID", 0, 65535, &val);
        if(status !=MENU_OK)
          break;
        config.id = val;
        WRITE_CFG(id);
      }
        break;
        case SYSTEM_MENU_PASSWORD:
        {
          int val = get_config_app()->password;
          status = input_decimal("Password", 0, 65535, &val);
          if (status != MENU_OK)
            break;
          config.password = val;
          WRITE_CFG(password);
        }
      break;
      case SYSTEM_MENU_CHARGER:
      {
        choice = get_config_app()->charger_model;
        status = input_combobox("Charger",g_charger_list_eng,_countof(g_charger_list_eng),&choice);
        if (status != MENU_OK)
          break;
        config.charger_model = (eCHARGER_MODEL_t)choice;
        WRITE_CFG(charger_model);
        show_popup("Information", "Applied after reset");
      }
      default:
        break;
      }

      if (status == MENU_ABORT)
      return status;
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}
