
#include <stdio.h>
#include "menu_data.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "menu_handler.h"
#include "util_time.h"
#include "app_dataLogging.h"
#include "old_aws_define.h"
#include "aws_data.h"
#include "app_key.h"
#include "view_driver.h"
#define DATA_MENU_AWS 0
#define DATA_WD 10


#define MENU_PAGE_PRINTF screen_printf_row

void draw_setup_data_menu(screen_menu_t *p_win)
{
  int32_t row_count = 0;

  screen_menu_start(p_win);

  screen_update_list(p_win, row_count, DATA_MENU_AWS);
  screen_menu_printf_row(p_win, row_count++, "%-*s", DATA_WD, "AWS");

  screen_menu_clear(p_win);
}

void draw_aws_data_page(screen_page_t *p_win, AWS_DATA_STRUCT *p_aws, uint32_t startTime)
{
  int32_t row_count = 0;
  DATE_TIME_BUF ct;

  screen_page_start(p_win);

  time_cvt_secTotime(startTime, &ct);

  MENU_PAGE_PRINTF(p_win, row_count++, "%04d-%02d-%02d %02d:%02d:00",ct.Year,ct.Month,ct.Day,ct.Hour,ct.Min);
  MENU_PAGE_PRINTF(p_win, row_count++, "TEMP       :%6.1f", READ_TEMP(p_aws->mTemperature.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "WIND DIR   :%6.1f", READ_X10(p_aws->mWind.mDirection.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "WIND SPEED :%6.1f", READ_X10(p_aws->mWind.mSpeed.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "GUST WIND D:%6.1f", READ_X10(p_aws->mWind.mDirection.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "GUST WIND S:%6.1f", READ_X10(p_aws->mWind.mSpeed.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "RAIN       :%6.1f", READ_X10(p_aws->mRainFall.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "BAROMETER  :%6.1f", READ_X10(p_aws->mBarometric.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "RAIN P     :%6d", p_aws->mRainDetect.sReal);
  MENU_PAGE_PRINTF(p_win, row_count++, "SNOW       :%6d", p_aws->mSnowFall.sReal);
  MENU_PAGE_PRINTF(p_win, row_count++, "HUMI       :%6.1f", READ_X10(p_aws->mHumidity.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOLAR R    :%7.2f", READ_X100(p_aws->mSolarRad.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOLAR D    :%6d", p_aws->mSunshine.sReal);
  MENU_PAGE_PRINTF(p_win, row_count++, "SOIL T 5cm :%6.1f", READ_TEMP(p_aws->mSoilTemp5cm.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOIL T 10cm:%6.1f", READ_TEMP(p_aws->mSoilTemp10cm.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOIL T 20cm:%6.1f", READ_TEMP(p_aws->mSoilTemp20cm.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOIL T 30cm:%6.1f", READ_TEMP(p_aws->mSoilTemp30cm.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOIL T 50cm:%6.1f", READ_TEMP(p_aws->mSoilTemp50cm.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOIL T   1m:%6.1f", READ_TEMP(p_aws->mSoilTemp1_0m.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOIL T 1.5m:%6.1f", READ_TEMP(p_aws->mSoilTemp1_5m.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOIL T 3.0m:%6.1f", READ_TEMP(p_aws->mSoilTemp3_0m.sReal));
  MENU_PAGE_PRINTF(p_win, row_count++, "SOIL T 5.0m:%6.1f", READ_TEMP(p_aws->mSoilTemp5_0m.sReal));

  p_win->total_items[0] = ALIGN_UP(row_count, p_win->view_row);

  screen_page_clear(p_win);
}

int32_t menu_data_aws(void)
{
  int32_t status;
  int32_t key;
  int year,month,day,hour,min;
  string_fmt_t strfmt;
  AWS_DATA_STRUCT aws;
  DATE_TIME_BUF nt;
  uint32_t startTime;
  screen_page_t lcd_win;


  screen_page_create(&lcd_win, 8, 20);
  lcd_win.total_pages = 1;
  lcd_win.chunk_scroll_use = 1;
  year = Date_Time.Year % 100;
  month = Date_Time.Month;
  day = Date_Time.Day;
  hour = Date_Time.Hour;
  min = Date_Time.Min;

  strfmt.fmt = "%02d-%02d %02d:%02d";
  snprintf(strfmt.data, sizeof(strfmt.data), strfmt.fmt, year,month,day,hour,min);
  status = input_fmt(&strfmt, "Start");
  if(status ==MENU_OK)
  {
    sscanf(strfmt.data, strfmt.fmt, &year, &month, &day,&hour,&min);
    nt.Year = year+2000;
    nt.Month = month;
    nt.Day = day;
    nt.Hour = hour;
    nt.Min = min;
    nt.Sec = 0;
    do
    {
      if(read_data_month(&nt, &aws, sizeof(aws), LOGGING_AWS, 1)>0)
      {
        show_popup("Information", "File Open Err");
        break;
      }
      startTime = SetTime(nt.Year, nt.Month,nt.Day, nt.Hour,nt.Min, 0);
      draw_aws_data_page(&lcd_win, &aws, startTime);
      screen_refresh();
      key = get_button_key(1000);

      if (key == KEY_CODE_CTRL_Q)
      {
        status = MENU_ABORT;
        break;
      }
      else if (key == KEY_CODE_CTRL_C)
      {
        status = MENU_BACK;
        break;
      }
      else if(key == KEY_CODE_RIGHT)
      {
        startTime +=60;
        time_cvt_secTotime(startTime, &nt);
      }
      else if(key == KEY_CODE_LEFT)
      {
        startTime -= 60;
        time_cvt_secTotime(startTime, &nt);
      }
      else if (key != KEY_CODE_NONE)
      {
        screen_page_handle(&lcd_win, key);
      }
    }while(1);

  }
  return status;
}

int32_t setup_menu_data(void)
{
  int32_t key;
  screen_menu_t menu;

  screen_menu_create(&menu, "Data");

  while (1)
  {
    draw_setup_data_menu(&menu);
    screen_refresh();

    key = get_button_key(1000);

    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
    else if (key == KEY_CODE_CTRL_C)
    {
      break;
    }
    if (key == KEY_CODE_ENTER)
    {
      screen_clear();
      switch(menu.selected_index)
      {
        case DATA_MENU_AWS:
          menu_data_aws();
           break;
      }
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}
