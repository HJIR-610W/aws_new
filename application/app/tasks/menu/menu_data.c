
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
#define DATA_MENU_1MIN_RAIN 1
#define DATA_MENU_1MIN_SOLAR_R 2

#define DATA_WD 10




void draw_data_menu(screen_menu_t *p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, DATA_MENU_AWS, "AWS");
  screen_menu_printf(p_win, DATA_MENU_1MIN_RAIN, "rain 1min");
  screen_menu_printf(p_win, DATA_MENU_1MIN_SOLAR_R, "sunshine 1min");
  screen_menu_clear(p_win);
}

void draw_aws_data_page(screen_page_t *p_win, AWS_DATA_STRUCT *p_aws, uint32_t start_time)
{
  DATE_TIME_BUF ct;

  screen_page_start(p_win);
  time_cvt_secTotime(start_time, &ct);
  screen_page_printf(p_win, "%04d-%02d-%02d %02d:%02d:00", ct.Year, ct.Month, ct.Day, ct.Hour, ct.Min);
  screen_page_printf(p_win, "TEMP       :%6.1f", READ_TEMP(p_aws->mTemperature.sReal));
  screen_page_printf(p_win, "WIND DIR   :%6.1f", READ_X10(p_aws->mWind.mDirection.sReal));
  screen_page_printf(p_win, "WIND SPEED :%6.1f", READ_X10(p_aws->mWind.mSpeed.sReal));
  screen_page_printf(p_win, "GUST WIND D:%6.1f", READ_X10(p_aws->mWind.mDirection.sReal));
  screen_page_printf(p_win, "GUST WIND S:%6.1f", READ_X10(p_aws->mWind.mSpeed.sReal));
  screen_page_printf(p_win, "RAIN       :%6.1f", READ_X10(p_aws->mRainFall.sReal));
  screen_page_printf(p_win, "BAROMETER  :%6.1f", READ_X10(p_aws->mBarometric.sReal));
  screen_page_printf(p_win, "RAIN P     :%6d", p_aws->mRainDetect.sReal);
  screen_page_printf(p_win, "SNOW       :%6d", p_aws->mSnowFall.sReal);
  screen_page_printf(p_win, "HUMI       :%6.1f", READ_X10(p_aws->mHumidity.sReal));
  screen_page_printf(p_win, "SOLAR R    :%7.2f", READ_X100(p_aws->mSolarRad.sReal));
  screen_page_printf(p_win, "SOLAR D    :%6d", p_aws->mSunshine.sReal);
  screen_page_printf(p_win, "SOIL T 5cm :%6.1f", READ_TEMP(p_aws->mSoilTemp5cm.sReal));
  screen_page_printf(p_win, "SOIL T 10cm:%6.1f", READ_TEMP(p_aws->mSoilTemp10cm.sReal));
  screen_page_printf(p_win, "SOIL T 20cm:%6.1f", READ_TEMP(p_aws->mSoilTemp20cm.sReal));
  screen_page_printf(p_win, "SOIL T 30cm:%6.1f", READ_TEMP(p_aws->mSoilTemp30cm.sReal));
  screen_page_printf(p_win, "SOIL T 50cm:%6.1f", READ_TEMP(p_aws->mSoilTemp50cm.sReal));
  screen_page_printf(p_win, "SOIL T   1m:%6.1f", READ_TEMP(p_aws->mSoilTemp1_0m.sReal));
  screen_page_printf(p_win, "SOIL T 1.5m:%6.1f", READ_TEMP(p_aws->mSoilTemp1_5m.sReal));
  screen_page_printf(p_win, "SOIL T 3.0m:%6.1f", READ_TEMP(p_aws->mSoilTemp3_0m.sReal));
  screen_page_printf(p_win, "SOIL T 5.0m:%6.1f", READ_TEMP(p_aws->mSoilTemp5_0m.sReal));
  screen_page_clear(p_win);
}


  int32_t menu_data_aws(void)
  {
    int32_t status;
    int32_t key;
    int  month, day, hour, min;
    uint32_t startTime;
    screen_page_t lcd_win;
    string_fmt_t strfmt;
    AWS_DATA_STRUCT aws;
    DATE_TIME_BUF nt;
    int32_t update=1;
    screen_clear();
    screen_page_create(&lcd_win, 8, 20);
    lcd_win.total_pages = 1;
    lcd_win.chunk_scroll_use = 1;

    month = Date_Time.Month;
    day = Date_Time.Day;
    hour = Date_Time.Hour;
    min = Date_Time.Min;

    strfmt.fmt = "%02d-%02d %02d:%02d";
    snprintf(strfmt.data, sizeof(strfmt.data), strfmt.fmt, month, day, hour, min);
    status = input_fmt(&strfmt, "MM/DD HH:MM");
    if (status == MENU_OK)
    {
      sscanf(strfmt.data, strfmt.fmt, &month, &day, &hour, &min);
      nt.Year = Date_Time.Year;
      nt.Month = month;
      nt.Day = day;
      nt.Hour = hour;
      nt.Min = min;
      nt.Sec = 0;
      do
      {
        if (update)
        {
          update = 0;
           if (read_data_month(&nt, &aws, sizeof(aws), LOGGING_AWS, 1) > 0)
          {
            show_popup("Information", "File Open Err");
            break;
          }
          startTime = SetTime(nt.Year, nt.Month, nt.Day, nt.Hour, nt.Min, 0);
        }
        draw_aws_data_page(&lcd_win, &aws, startTime);
        screen_refresh();

        key = get_button_key(0xFFFFFFFF);

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
        else if (key == KEY_CODE_RIGHT)
        {
          startTime += 60;
          time_cvt_secTotime(startTime, &nt);
          update = 1;
        }
        else if (key == KEY_CODE_LEFT)
        {
          startTime -= 60;
          time_cvt_secTotime(startTime, &nt);
          update = 1;
        }
        else if (key != KEY_CODE_UNKNOWN)
        {

          screen_page_handle(&lcd_win, key);
        }
      } while (1);
    }
    return status;
  }

  #define MIN_VIEW_ROW 7
  //24-05-00 00:00 1254
  void draw_1min_page(screen_page_t *p_win, uint16_t data[MIN_VIEW_ROW], uint32_t start_time,int system)
  {
    float value;
    DATE_TIME_BUF ct;
    uint32_t base_time = start_time-60*MIN_VIEW_ROW;
    screen_page_start(p_win);

    switch (system)
    {
    case LOGGING_RAIN_1MIN:
      screen_page_printf(p_win, "%s", "Rain(1min)");
      break;
    case LOGGING_SUNSHINE_1MIN:
      screen_page_printf(p_win, "%s", "Sunshine(1min)");
      break;
    
    default:
      screen_page_printf(p_win, "%s", "Unknown");
      return;
    }


    for (int i = 0; i < MIN_VIEW_ROW; i++)
    {
      time_cvt_secTotime(base_time, &ct);

      switch (system)
      {
      case LOGGING_RAIN_1MIN:
        value = (float)data[i] / 10.0f;
        screen_page_printf(p_win, "%02d-%02d-%02d %02d:%02d %6.1fmm", ct.Year % 100, ct.Month, ct.Day, ct.Hour, ct.Min,value );
        break;
      case LOGGING_SUNSHINE_1MIN:
        screen_page_printf(p_win, "%02d-%02d-%02d %02d:%02d %2dsec", ct.Year % 100, ct.Month, ct.Day, ct.Hour, ct.Min, data[i]);
        break;
      }

        base_time += 60;
      }
    screen_page_clear(p_win);
  }



  int32_t menu_view_1min(int system)
  {
    int32_t status;
    int32_t key;
    int32_t  month, day, hour, min;
    uint32_t startTime;
    screen_page_t lcd_win;
    string_fmt_t strfmt;
    DATE_TIME_BUF nt;
    int update=1;
    uint16_t rain[MIN_VIEW_ROW];
    screen_clear();
    screen_page_create(&lcd_win, 8, 20);
    lcd_win.total_pages = 1;
    lcd_win.chunk_scroll_use = 1;

    month = Date_Time.Month;
    day = Date_Time.Day;
    hour = Date_Time.Hour;
    min = Date_Time.Min;

    strfmt.fmt = "%02d-%02d %02d:%02d";
    snprintf(strfmt.data, sizeof(strfmt.data), strfmt.fmt, month, day, hour, min);
    status = input_fmt(&strfmt, "MM/DD HH:MM");
    if (status == MENU_OK)
    {
      sscanf(strfmt.data, strfmt.fmt, &month, &day, &hour, &min);
      nt.Year = Date_Time.Year;
      nt.Month = month;
      nt.Day = day;
      nt.Hour = hour;
      nt.Min = min;
      nt.Sec = 0;


      do
      {
        if (update)
        {
          update = 0;
          for (int i = 0; i < MIN_VIEW_ROW; i++)
          {
            read_sensorDataMulti(&nt, sizeof(uint16_t), 1, system, 1, (uint8_t *)&rain[i], 2);
            startTime = SetTime(nt.Year, nt.Month, nt.Day, nt.Hour, nt.Min, 0);
            startTime += 60;
            time_cvt_secTotime(startTime, &nt);
          }
          draw_1min_page(&lcd_win,rain, startTime,system);
          screen_refresh();
        }
        key = get_button_key(0xFFFFFFFF);

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
        else if (key == KEY_CODE_UP)
        {
          startTime -= 60 * MIN_VIEW_ROW * 2;
          time_cvt_secTotime(startTime, &nt);
          update = 1;
        }
        else if(key == KEY_CODE_DOWN)
        {
          update = 1;
        }
      } while (1);
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
      draw_data_menu(&menu);
      screen_refresh();

      key = get_button_key(1000);

      if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
      {
        break;
      }
      if (key == KEY_CODE_ENTER)
      {
        switch (menu.index_list[menu.selected_index])
        {
          case DATA_MENU_AWS:
            menu_data_aws();
           break;
          case DATA_MENU_1MIN_RAIN:
            menu_view_1min(LOGGING_RAIN_1MIN);
             break;
          case DATA_MENU_1MIN_SOLAR_R:
            menu_view_1min(LOGGING_SUNSHINE_1MIN);
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

  return convert_key_to_status(key);
}
