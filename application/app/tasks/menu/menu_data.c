
#include <stdio.h>
#include <string.h>

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
#include "rain_data.h"
#include "sunshine_data.h"
#include "util_crc16_ccitt.h"
#include "FreeRTOS.h"
#include "utile_data.h"
#include "task_menu_define.h"

#define DATA_RAIN_1MIN 0
#define DATA_RAIN_INIT 1
#define DATA_RAIN_EDIT 2

void draw_data_rain(screen_menu_t *p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, DATA_RAIN_1MIN, "1min data");
  screen_menu_printf(p_win, DATA_RAIN_INIT, "Reset To Zero");
  screen_menu_printf(p_win, DATA_RAIN_EDIT, "Edit");
  screen_menu_clear(p_win);
}

const osThreadAttr_t kEraseTask_attributes = {
    .name = "erase",
    .stack_size = TASK_STACK(TASK_FILE_ERASE_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_FILE_ERASE_DEF),
};


typedef struct file_erase_s
{
  void *task_handle;
  uint8_t file_type;
}file_erase_t;

#define FILE_RAIN 0
#define FILE_SUNSHINE 1




void fileEraseTask( void *arg)
{
  file_erase_t *p_erase = (file_erase_t *)arg;
  int32_t result;
  uint32_t flags = TASK_MENU_ALARM_FILE_RET_FAIL; // 에러

  if(p_erase->file_type == FILE_RAIN)
  {
  result = rain_file_zero(Date_Time.Year);
  }
  else if(p_erase ->file_type ==FILE_SUNSHINE)
  {
  result = sunshine_file_zero(Date_Time.Year) ;
  }

  if(result ==0)
  {
    flags = TASK_MENU_ALARM_FILE_RET_OK; // 정상
  }
  osThreadFlagsSet(p_erase->task_handle, flags);
  osThreadExit();
}


int32_t setup_data_erase(uint8_t file_type)
{
  static file_erase_t erase;
  int32_t status;
  int32_t choice = 0;
  int32_t count=0;
  uint32_t flags;

  status = input_active("Initialize all to 0?", &choice);
    
  if(status != MENU_OK)
  {
    return status;
  }
  
  if( choice == 1)
  {
    erase.task_handle = osThreadGetId();
    erase.file_type = file_type;
    osThreadFlagsClear(TASK_MENU_ALARM_FILE_RET_OK | TASK_MENU_ALARM_FILE_RET_FAIL);
    osThreadNew(fileEraseTask, &erase, &kEraseTask_attributes);
  }

screen_clear();
screen_printf(6, 0, " ");

while (1)
{
  char buffer[20];
  buffer[count++] = '*';
  buffer[count] = 0;

  screen_printf(6, 0, buffer);

  if (count == 10)
  {
    memset(buffer, ' ', sizeof(buffer));
    buffer[sizeof(buffer) - 1] = 0;
    screen_printf(6, 0, buffer);
    count = 0;
  }
  screen_refresh();
  screen_refresh();
  flags = osThreadFlagsWait(TASK_MENU_ALARM_FILE_RET_OK | TASK_MENU_ALARM_FILE_RET_FAIL, osFlagsWaitAny, 50);

  if((flags &0x80000000)==0)//음수가 아니어야 한다
  {
  if (flags & TASK_MENU_ALARM_FILE_RET_OK || flags & TASK_MENU_ALARM_FILE_RET_FAIL)
  {
    break;
  }
  }
}

if (flags &TASK_MENU_ALARM_FILE_RET_OK)
{
  show_popup("Information", "Completed");
  if(erase.file_type ==FILE_RAIN)
  calculate_rain();
  else if(erase.file_type == FILE_SUNSHINE)
    calculate_sunshine();
}
else
{
  show_popup("Information", "Failed to complete");
}

  return status;
}

int32_t edit_file(uint8_t file_type)
{
  int32_t year, month, day, hour, min;
  int32_t end_year, end_month, end_day, end_hour, end_min;
  int32_t status;
  int32_t value=0;
  int32_t ret=0;
  string_fmt_t strfmt;
  DATE_TIME_BUF start_time;
  DATE_TIME_BUF end_time;
  uint32_t start_stamp;
  uint32_t end_stamp;

  
  (void)ret;
  year = Date_Time.Year;
  month = Date_Time.Month;
  day = Date_Time.Day;
  hour = Date_Time.Hour;
  min = Date_Time.Min;

  strfmt.fmt = "%04d-%02d-%02d %02d:%02d";
  snprintf(strfmt.data, sizeof(strfmt.data), strfmt.fmt,year, month, day, hour, min);
  status = input_fmt(&strfmt, "Start Year-Month-Dday Hour:Min");
  if (status != MENU_OK)
    return status;

  sscanf(strfmt.data, strfmt.fmt, &year,&month, &day, &hour, &min);

  start_time.Year = year;
  start_time.Month = month;
  start_time.Day = day;
  start_time.Hour = hour;
  start_time.Min = min;
  start_time.Sec = 0;

  start_stamp = time_cvt_timestamp(&start_time);

  end_year = Date_Time.Year;
  end_month = Date_Time.Month;
  end_day = Date_Time.Day;
  end_hour = Date_Time.Hour;
  end_min = Date_Time.Min;

  strfmt.fmt = "%04d-%02d-%02d %02d:%02d";
  snprintf(strfmt.data, sizeof(strfmt.data), strfmt.fmt, end_year, end_month, end_day, end_hour, end_min);
  status = input_fmt(&strfmt, "End Year-Month-Dday Hour:Min");
  if (status != MENU_OK)
    return status;

  sscanf(strfmt.data, strfmt.fmt, &end_year, &end_month, &end_day, &end_hour, &end_min);

  end_time.Year = end_year;
  end_time.Month = end_month;
  end_time.Day = end_day;
  end_time.Hour = end_hour;
  end_time.Min = end_min;
  end_time.Sec = 0;

  end_stamp = time_cvt_timestamp(&end_time);

  if(start_stamp>end_stamp)
  {
    show_popup("Warnning","Verify input data");
    return MENU_OK;
  }

  status = input_decimal("Value",0,9999,&value);
  if (status != MENU_OK)
    return status;

  if (file_type == FILE_RAIN)
    ret = write_bulk_data_range(RAIN_1MIN_FILE_NAME, &start_time, &end_time, value);
    else if(file_type == FILE_SUNSHINE)
      ret = write_bulk_data_range(SUNSHINE_1MIN_FILE_NAME, &start_time, &end_time, value);

  show_popup("Information", "Completed");
  
  return MENU_OK;
}

#define DATA_SUNSHINE_1MIN 0
#define DATA_SUNSHINE_INIT 1
#define DATA_SUNSHINE_EDIT 2

void draw_data_sunshine(screen_menu_t *p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, DATA_RAIN_1MIN, "1min data");
  screen_menu_printf(p_win, DATA_RAIN_INIT, "Reset To Zero");
  screen_menu_printf(p_win, DATA_SUNSHINE_EDIT, "Edit");
  screen_menu_clear(p_win);
}


#define DATA_MENU_AWS     0
#define DATA_MENU_RAIN    1
#define DATA_MENU_SOLAR_R 2

#define DATA_WD 10


void draw_data_menu(screen_menu_t *p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, DATA_MENU_AWS, "AWS");
  screen_menu_printf(p_win, DATA_MENU_RAIN, "Rain");
  screen_menu_printf(p_win, DATA_MENU_SOLAR_R, "Sunshine");
  screen_menu_clear(p_win);
}

char *generate_data_fmt(char *buff,int buff_size,uint16_t data,const char *format, ...)
{
  va_list args;

  if(data==AWS_SEN_ERR)
  {
    snprintf(buff,buff_size,"ERROR");
  }
  else
  {
  va_start(args, format);
  vsnprintf(buff, buff_size, format, args);
  va_end(args);
  }

  return buff;
}
void draw_aws_data_page(screen_page_t *p_win, AWS_DATA_STRUCT *p_aws, uint32_t start_time)
{
  DATE_TIME_BUF ct;
  uint16_t crc;
  uint16_t data;
  char buff[30];

  const char *sensor_error = "ERROR";

  screen_page_start(p_win);
  time_cvt_secTotime(start_time, &ct);

  crc = crc16_ccitt_table((uint8_t *)p_aws, sizeof(AWS_DATA_STRUCT) - sizeof(uint16_t));
  if (crc == p_aws->crc)
  {
    screen_page_printf(p_win, "%04d-%02d-%02d %02d:%02d:00", ct.Year, ct.Month, ct.Day, ct.Hour, ct.Min);

    // TEMPERATURE
    strcpy(buff, "ERROR");
    if (p_aws->mTemperature.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mTemperature.sReal));
    }
    screen_page_printf(p_win, "TEMP       :%s", buff);

    // WIND DIRECTION
    strcpy(buff, "ERROR");
    if (p_aws->mWind.mDirection.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mWind.mDirection.sReal));
    }
    screen_page_printf(p_win, "WIND DIR   :%s", buff);

    // WIND SPEED
    strcpy(buff, "ERROR");
    if (p_aws->mWind.mSpeed.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mWind.mSpeed.sReal));
    }
    screen_page_printf(p_win, "WIND SPEED :%s", buff);

    // WIND GUST DIRECTION
    strcpy(buff, "ERROR");
    if (p_aws->mWind.mDirection.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mWind.mDirection.sReal));
    }
    screen_page_printf(p_win, "WIND GUST D:%s", buff);

    // WIND GUST SPEED
    strcpy(buff, "ERROR");
    if (p_aws->mWind.mSpeed.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mWind.mSpeed.sReal));
    }
    screen_page_printf(p_win, "WIND BUST S:%s", buff);

    // RAINFALL
    strcpy(buff, "ERROR");
    if (p_aws->mRainFall.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mRainFall.sReal));
    }
    screen_page_printf(p_win, "RAIN       :%s", buff);

    // BAROMETER
    strcpy(buff, "ERROR");
    if (p_aws->mBarometric.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mBarometric.sReal));
    }
    screen_page_printf(p_win, "BAROMETER  :%s", buff);

    // RAIN DETECT
    strcpy(buff, "ERROR");
    if (p_aws->mRainDetect.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6d", p_aws->mRainDetect.sReal);
    }
    screen_page_printf(p_win, "RAIN P     :%s", buff);

    // SNOWFALL
    strcpy(buff, "ERROR");
    if (p_aws->mSnowFall.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6d", p_aws->mSnowFall.sReal);
    }
    screen_page_printf(p_win, "SNOW       :%s", buff);

    // HUMIDITY
    strcpy(buff, "ERROR");
    if (p_aws->mHumidity.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mHumidity.sReal));
    }
    screen_page_printf(p_win, "HUMI       :%s", buff);

    // SOLAR RADIATION
    strcpy(buff, "ERROR");
    if (p_aws->mSolarRad.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%7.2f", READ_X100(p_aws->mSolarRad.sReal));
    }
    screen_page_printf(p_win, "SOLAR R    :%s", buff);

    // SUNSHINE DURATION
    strcpy(buff, "ERROR");
    if (p_aws->mSunshine.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6d", p_aws->mSunshine.sMax);
    }
    screen_page_printf(p_win, "SOLAR D    :%s", buff);

    // SOIL TEMPERATURE 5cm
    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp5cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp5cm.sReal));
    }
    screen_page_printf(p_win, "SOIL T 5cm :%s", buff);

    // SOIL TEMPERATURE 10cm
    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp10cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp10cm.sReal));
    }
    screen_page_printf(p_win, "SOIL T 10cm:%s", buff);

    // SOIL TEMPERATURE 20cm
    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp20cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp20cm.sReal));
    }
    screen_page_printf(p_win, "SOIL T 20cm:%s", buff);

    // SOIL TEMPERATURE 30cm
    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp30cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp30cm.sReal));
    }
    screen_page_printf(p_win, "SOIL T 30cm:%s", buff);

    // SOIL TEMPERATURE 50cm
    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp50cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp50cm.sReal));
    }
    screen_page_printf(p_win, "SOIL T 50cm:%s", buff);

    // SOIL TEMPERATURE 1m
    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp1_0m.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp1_0m.sReal));
    }
    screen_page_printf(p_win, "SOIL T   1m:%s", buff);

    // SOIL TEMPERATURE 1.5m
    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp1_5m.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp1_5m.sReal));
    }
    screen_page_printf(p_win, "SOIL T 1.5m:%s", buff);

    // SOIL TEMPERATURE 3.0m
    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp3_0m.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp3_0m.sReal));
    }
    screen_page_printf(p_win, "SOIL T 3.0m:%s", buff);

    // SOIL TEMPERATURE 5.0m
    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp5_0m.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp5_0m.sReal));
    }
    screen_page_printf(p_win, "SOIL T 5.0m:%s", buff);

    // SOLAR VOLTAGE - 이 값들은 에러 체크 대상이 아닐 수 있음
    screen_page_printf(p_win, "SOLAR Volt :%6.2f", (float)p_aws->solar_m_voltage / 1000.0);
    screen_page_printf(p_win, "BATT Volt  :%6.2f", (float)p_aws->battery_m_voltage / 1000.0);
  }
  else
  {
    screen_page_printf(p_win, "%04d-%02d-%02d %02d:%02d:00", ct.Year, ct.Month, ct.Day, ct.Hour, ct.Min);
    screen_page_printf(p_win, "No data saved");
  }
  screen_page_clear(p_win);
}

  int32_t menu_data_aws(void)
  {
    int32_t status;
    int32_t key;
    int32_t year,month, day, hour, min;
    int32_t update = 1;
    uint32_t startTime;
    screen_page_t lcd_win;
    string_fmt_t strfmt;
    AWS_DATA_STRUCT aws;
    DATE_TIME_BUF nt;

    screen_clear();
    screen_page_create(&lcd_win);
    
    lcd_win.total_pages = 1;
    lcd_win.chunk_scroll_enable = 1;

    year = Date_Time.Year;
    month = Date_Time.Month;
    day = Date_Time.Day;
    hour = Date_Time.Hour;
    min = Date_Time.Min;


    strfmt.fmt = "%04d-%02d-%02d %02d:%02d";
    while(1)
    {
    snprintf(strfmt.data, sizeof(strfmt.data), strfmt.fmt,year, month, day, hour, min);
    status = input_fmt(&strfmt, "Y-M-D H:M");
    if (status != MENU_OK)
    break; 
     
    sscanf(strfmt.data, strfmt.fmt,&year, &month, &day, &hour, &min);
      nt.Year = year;
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
            return MENU_OK;
          }
          startTime = SetTime(nt.Year, nt.Month, nt.Day, nt.Hour, nt.Min, 0);
        }
        draw_aws_data_page(&lcd_win, &aws, startTime);
        screen_refresh();

        key = get_menu_key(WAIT_FOREVER);

        if(key == KEY_CODE_CTRL_Q)
        {
           return convert_key_to_status(key);
        }
           
        if (key == KEY_CODE_CTRL_C)
        {
          break;
        }

        if (key == KEY_CODE_RIGHT)
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

    return status;//convert_key_to_status(key);
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
    int32_t  year,month, day, hour, min;
    uint32_t startTime;
    screen_page_t lcd_win;
    string_fmt_t strfmt;
    DATE_TIME_BUF nt;
    int update=1;
    uint16_t rain[MIN_VIEW_ROW];

    screen_clear();
    screen_page_create(&lcd_win);


    lcd_win.total_pages = 1;
    lcd_win.chunk_scroll_enable = 1;

    year = Date_Time.Year;
    month = Date_Time.Month;
    day = Date_Time.Day;
    hour = Date_Time.Hour;
    min = Date_Time.Min;

    strfmt.fmt = "%04d-%02d-%02d %02d:%02d";
    snprintf(strfmt.data, sizeof(strfmt.data), strfmt.fmt, year,month, day, hour, min);
    status = input_fmt(&strfmt, "Y-M-D H:M");
    if (status != MENU_OK)
    return status;

    {
      sscanf(strfmt.data, strfmt.fmt,&year, &month, &day, &hour, &min);
      nt.Year = year;
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
        key = get_menu_key(WAIT_FOREVER);

        if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
        {
          break;
        }

        if (key == KEY_CODE_UP)
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
    return convert_key_to_status(key);
  }

  int32_t setup_menu_data_rain(void)
  {
    int32_t key;
    int32_t status;
    screen_menu_t menu;

    screen_menu_create(&menu, "Rain Data");

    while (1)
    {
      draw_data_rain(&menu);
      screen_refresh();

      key = get_menu_key(WAIT_FOREVER);

      if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
      {
        break;
      }
      if (key == KEY_CODE_ENTER)
      {
        switch (menu.index_list[menu.selected_index])
        {
        case DATA_RAIN_1MIN:
          status = menu_view_1min(LOGGING_RAIN_1MIN);
          break;
        case DATA_RAIN_INIT:
          status = setup_data_erase(FILE_RAIN);
          break;
        case DATA_RAIN_EDIT:
          status = edit_file(FILE_RAIN);
           break;

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

    int32_t setup_menu_sunshine(void)
  {
    int32_t key;
    int32_t status;
    screen_menu_t menu;

    screen_menu_create(&menu, "Sunshine Data");

    while (1)
    {
      draw_data_sunshine(&menu);
      screen_refresh();

      key = get_menu_key(WAIT_FOREVER);

      if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
      {
        break;
      }
      if (key == KEY_CODE_ENTER)
      {
        switch (menu.index_list[menu.selected_index])
        {
        case DATA_SUNSHINE_1MIN:
          status = menu_view_1min(LOGGING_SUNSHINE_1MIN);
          break;
        case DATA_SUNSHINE_INIT:
           status = setup_data_erase(FILE_SUNSHINE);
               break;
        case DATA_SUNSHINE_EDIT:
          status = edit_file(FILE_SUNSHINE);
           break;
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
  int32_t setup_menu_data(void)
  {
    int32_t key;
    int32_t status;
    screen_menu_t menu;

    screen_menu_create(&menu, "Data");

    while (1)
    {
      draw_data_menu(&menu);
      screen_refresh();

      key = get_menu_key(WAIT_FOREVER);

      if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
      {
        break;
      }
      if (key == KEY_CODE_ENTER)
      {
        switch (menu.index_list[menu.selected_index])
        {
          case DATA_MENU_AWS:
            status = menu_data_aws();
           break;
          case DATA_MENU_RAIN:
            status = setup_menu_data_rain();
            break;
          case DATA_MENU_SOLAR_R:
            status = setup_menu_sunshine();
            break;
        default:
          break;
        }
        if(status == MENU_ABORT)
        return status;
      }
      else if (key != KEY_CODE_UNKNOWN)
      {
        screen_menu_handle(&menu, key);
      }
  }

  return convert_key_to_status(key);
}
