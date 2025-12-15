#include "console_menu_data.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "app_dataLogging.h"
#include "aws_data.h"
#include "config_app.h"
#include "console_define.h"
#include "console_utile.h"
#include "debug_io.h"
#include "old_aws_define.h"
#include "rain_data.h"
#include "sunshine_data.h"
#include "task_menu_define.h"
#include "util_crc16_ccitt.h"
#include "util_memory.h"
#include "util_time.h"
#include "utile_data.h"
#include "const_string.h"

#define FILE_RAIN 0
#define FILE_SUNSHINE 1

const osThreadAttr_t kEraseTask_atts = {
    .name = "erase",
    .stack_size = TASK_STACK(TASK_FILE_ERASE_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_FILE_ERASE_DEF),
};

typedef struct file_erase_s
{
  void *task_handle;
  uint8_t file_type;
} file_erase_t;

void fileEraseTask2(void *arg)
{
  int32_t result=1;
  uint32_t flags = TASK_MENU_ALARM_FILE_RET_FAIL;
  file_erase_t *p_erase = (file_erase_t *)arg;

  if (p_erase->file_type == FILE_RAIN)
  {
    result = rain_file_zero(Date_Time.Year);
  }
  else if (p_erase->file_type == FILE_SUNSHINE)
  {
    result = sunshine_file_zero(Date_Time.Year);
  }

  if (result == 0)
  {
    flags = TASK_MENU_ALARM_FILE_RET_OK;
  }
  osThreadFlagsSet(p_erase->task_handle, flags);
  osThreadExit();
}

int32_t aws_data_erase(uint8_t file_type)
{
  int32_t choice = 0;
  int32_t status;
  int32_t count = 0;
  uint32_t flags;
  static file_erase_t erase;

  status = view_input_active("모든 데이터를 0으로 초기화", &choice);

  if (status != MENU_OK)
  {
    return status;
  }

  if (choice == 1)
  {
    erase.task_handle = osThreadGetId();
    erase.file_type = file_type;
    osThreadFlagsClear(TASK_MENU_ALARM_FILE_RET_OK | TASK_MENU_ALARM_FILE_RET_FAIL);
    osThreadNew(fileEraseTask2, &erase, &kEraseTask_atts);

    debug_printf("\r\n처리 중");

    while (1)
    {
      char buffer[20];

      buffer[count++] = '*';
      buffer[count] = 0;

      debug_printf("\r처리 중%s", buffer);

      if (count == 10)
      {
        count = 0;
      }

      flags = osThreadFlagsWait(TASK_MENU_ALARM_FILE_RET_OK | TASK_MENU_ALARM_FILE_RET_FAIL, osFlagsWaitAny, 50);

      if ((flags & 0x80000000) == 0)
      {
        if (flags & TASK_MENU_ALARM_FILE_RET_OK || flags & TASK_MENU_ALARM_FILE_RET_FAIL)
        {
          break;
        }
      }
    }

    debug_printf("\r\n");

    if (flags & TASK_MENU_ALARM_FILE_RET_OK)
    {
      debug_printf_color(IO_COLOR_GREEN, "완료\r\n");
      if (erase.file_type == FILE_RAIN)
        calculate_rain();
      else if (erase.file_type == FILE_SUNSHINE)
        calculate_sunshine();
    }
    else
    {
      debug_printf_color(IO_COLOR_RED, "실패\r\n");
    }
  }

  return status;
}

int32_t aws_edit_file(uint8_t file_type)
{
  int32_t year, month, day, hour, min;
  int32_t end_year, end_month, end_day, end_hour, end_min;
  int32_t status;
  int32_t value = 0;
  int32_t ret = 0;
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

  debug_printf("시작 시간 (YYYY-MM-DD HH:MM)\r\n");
  status = debug_scanf_s("%04d-%02d-%02d %02d:%02d", &year, &month, &day, &hour, &min);

  if (status == KEY_CODE_CTRL_C)
  {
    return MENU_BACK;
  }
  if (status == KEY_CODE_CTRL_P)
  {
    return MENU_ABORT;
  }
  if (status != 5)
  {
    debug_printf("입력을 확인해주세요\r\n");
    return MENU_OK;
  }

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

  debug_printf("종료 시간 (YYYY-MM-DD HH:MM)\r\n");
  status = debug_scanf_s("%04d-%02d-%02d %02d:%02d", &end_year, &end_month, &end_day, &end_hour, &end_min);

  if (status == KEY_CODE_CTRL_C)
  {
    return MENU_BACK;
  }
  if (status == KEY_CODE_CTRL_P)
  {
    return MENU_ABORT;
  }
  if (status != 5)
  {
    debug_printf("입력을 확인해주세요\r\n");
    return MENU_OK;
  }

  end_time.Year = end_year;
  end_time.Month = end_month;
  end_time.Day = end_day;
  end_time.Hour = end_hour;
  end_time.Min = end_min;
  end_time.Sec = 0;

  end_stamp = time_cvt_timestamp(&end_time);

  if (start_stamp > end_stamp)
  {
    debug_printf_color(IO_COLOR_RED, "입력 데이터를 확인해주세요\r\n");
    return MENU_OK;
  }

  status = view_input_decimal("값", &value, 0, 9999);
  if (status != MENU_OK)
    return status;

  if (file_type == FILE_RAIN)
    ret = write_bulk_data_range(RAIN_1MIN_FILE_NAME, &start_time, &end_time, value);
  else if (file_type == FILE_SUNSHINE)
    ret = write_bulk_data_range(SUNSHINE_1MIN_FILE_NAME, &start_time, &end_time, value);

  debug_printf_color(IO_COLOR_GREEN, "완료\r\n");

  return MENU_OK;
}

void aws_display_aws_data(AWS_DATA_STRUCT *p_aws, uint32_t start_time)
{
  char buff[30];
  uint16_t crc;
  DATE_TIME_BUF ct;

  time_cvt_secTotime(start_time, &ct);

  crc = crc16_ccitt_table((uint8_t *)p_aws, sizeof(AWS_DATA_STRUCT) - sizeof(uint16_t));

  debug_printf("\r\n");
  debug_printf("========================================\r\n");
  debug_printf("시간: %04d-%02d-%02d %02d:%02d:00\r\n", ct.Year, ct.Month, ct.Day, ct.Hour, ct.Min);

  if (crc == p_aws->crc)
  {
    strcpy(buff, "ERROR");
    if (p_aws->mTemperature.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mTemperature.sReal));
    }
    debug_printf("기온           :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mWind.mDirection.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mWind.mDirection.sReal));
    }
    debug_printf("풍향           :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mWind.mSpeed.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mWind.mSpeed.sReal));
    }
    debug_printf("풍속           :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mRainFall.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mRainFall.sReal));
    }
    debug_printf("강우량         :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mBarometric.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mBarometric.sReal));
    }
    debug_printf("기압           :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mRainDetect.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6d", p_aws->mRainDetect.sReal);
    }
    debug_printf("강우 감지      :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSnowFall.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6d", p_aws->mSnowFall.sReal);
    }
    debug_printf("적설           :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mHumidity.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_X10(p_aws->mHumidity.sReal));
    }
    debug_printf("습도           :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSolarRad.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%7.2f", READ_X100(p_aws->mSolarRad.sReal));
    }
    debug_printf("일사           :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSunshine.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6d", p_aws->mSunshine.sMax);
    }
    debug_printf("일조 시간      :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp5cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp5cm.sReal));
    }
    debug_printf("지중온도  5cm  :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp10cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp10cm.sReal));
    }
    debug_printf("지중온도 10cm  :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp20cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp20cm.sReal));
    }
    debug_printf("지중온도 20cm  :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp30cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp30cm.sReal));
    }
    debug_printf("지중온도 30cm  :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp50cm.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp50cm.sReal));
    }
    debug_printf("지중온도 50cm  :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp1_0m.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp1_0m.sReal));
    }
    debug_printf("지중온도  1m   :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp1_5m.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp1_5m.sReal));
    }
    debug_printf("지중온도 1.5m  :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp3_0m.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp3_0m.sReal));
    }
    debug_printf("지중온도 3.0m  :%s\r\n", buff);

    strcpy(buff, "ERROR");
    if (p_aws->mSoilTemp5_0m.sReal != AWS_SEN_ERR)
    {
      snprintf(buff, sizeof(buff), "%6.1f", READ_TEMP(p_aws->mSoilTemp5_0m.sReal));
    }
    debug_printf("지중온도 5.0m  :%s\r\n", buff);

    debug_printf("태양전지 전압  :%6.2f\r\n", (float)p_aws->solar_m_voltage / 1000.0);
    debug_printf("배터리 전압    :%6.2f\r\n", (float)p_aws->battery_m_voltage / 1000.0);
  }
  else
  {
    debug_printf("저장된 데이터 없음\r\n");
  }
  debug_printf("========================================\r\n");
}

int32_t aws_view_aws_data(void)
{
  int32_t year, month, day, hour, min;
  int32_t status;
  uint32_t startTime;
  DATE_TIME_BUF nt;
  AWS_DATA_STRUCT aws;

  year = Date_Time.Year;
  month = Date_Time.Month;
  day = Date_Time.Day;
  hour = Date_Time.Hour;
  min = Date_Time.Min;

  debug_printf("조회 시간 (YYYY-MM-DD HH:MM)\r\n");
  status = debug_scanf_s("%04d-%02d-%02d %02d:%02d", &year, &month, &day, &hour, &min);

  if (status == KEY_CODE_CTRL_C)
  {
    return MENU_BACK;
  }
  if (status == KEY_CODE_CTRL_P)
  {
    return MENU_ABORT;
  }
  if (status != 5)
  {
    debug_printf("입력을 확인해주세요\r\n");
    return MENU_OK;
  }

  nt.Year = year;
  nt.Month = month;
  nt.Day = day;
  nt.Hour = hour;
  nt.Min = min;
  nt.Sec = 0;

  if (read_data_month(&nt, &aws, sizeof(aws), LOGGING_AWS, 1) > 0)
  {
    debug_printf_color(IO_COLOR_RED, "파일 열기 실패\r\n");
    return MENU_OK;
  }

  startTime = SetTime(nt.Year, nt.Month, nt.Day, nt.Hour, nt.Min, 0);
  aws_display_aws_data(&aws, startTime);

  return MENU_OK;
}

#define MIN_VIEW_ROW 10

void aws_display_1min_data(uint16_t data[MIN_VIEW_ROW], uint32_t start_time, int system)
{
  char buff[30];
  float value;
  DATE_TIME_BUF ct;
  uint32_t base_time = start_time - 60 * MIN_VIEW_ROW;

  debug_printf("\r\n");
  debug_printf("========================================\r\n");

  switch (system)
  {
    case LOGGING_RAIN_1MIN:
      debug_printf("강우량 (1분)\r\n");
      break;
    case LOGGING_SUNSHINE_1MIN:
      debug_printf("일조 시간 (1분)\r\n");
      break;
    default:
      debug_printf("알 수 없는 데이터\r\n");
      return;
  }

  debug_printf("----------------------------------------\r\n");

  for (int i = 0; i < MIN_VIEW_ROW; i++)
  {
    time_cvt_secTotime(base_time, &ct);

    switch (system)
    {
      case LOGGING_RAIN_1MIN:
        value = (float)data[i] / 10.0f;
        debug_printf("%02d-%02d-%02d %02d:%02d  %6.1fmm\r\n",
                     ct.Year % 100, ct.Month, ct.Day, ct.Hour, ct.Min, value);
        break;
      case LOGGING_SUNSHINE_1MIN:
        debug_printf("%02d-%02d-%02d %02d:%02d  %2dsec\r\n",
                     ct.Year % 100, ct.Month, ct.Day, ct.Hour, ct.Min, data[i]);
        break;
    }

    base_time += 60;
  }
  debug_printf("========================================\r\n");
}

int32_t aws_view_1min_data(int system)
{
  int32_t year, month, day, hour, min;
  int32_t status;
  uint16_t data[MIN_VIEW_ROW];
  uint32_t startTime=0;
  DATE_TIME_BUF nt;

  year = Date_Time.Year;
  month = Date_Time.Month;
  day = Date_Time.Day;
  hour = Date_Time.Hour;
  min = Date_Time.Min;

  debug_printf("조회 시간 (YYYY-MM-DD HH:MM)\r\n");
  status = debug_scanf_s("%04d-%02d-%02d %02d:%02d", &year, &month, &day, &hour, &min);

  if (status == KEY_CODE_CTRL_C)
  {
    return MENU_BACK;
  }
  if (status == KEY_CODE_CTRL_P)
  {
    return MENU_ABORT;
  }
  if (status != 5)
  {
    debug_printf("입력을 확인해주세요\r\n");
    return MENU_OK;
  }

  nt.Year = year;
  nt.Month = month;
  nt.Day = day;
  nt.Hour = hour;
  nt.Min = min;
  nt.Sec = 0;

  for (int i = 0; i < MIN_VIEW_ROW; i++)
  {
    read_sensorDataMulti(&nt, sizeof(uint16_t), 1, system, 1, (uint8_t *)&data[i], 2);
    startTime = SetTime(nt.Year, nt.Month, nt.Day, nt.Hour, nt.Min, 0);
    startTime += 60;
    time_cvt_secTotime(startTime, &nt);
  }

  aws_display_1min_data(data, startTime, system);

  return MENU_OK;
}

int32_t aws_menu_data_rain(void)
{
  char buff[3][40];
  const char* menu[3];
  int choice, status;
  int menu_cnt;

  for (int i = 0; i < 3; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "1분 데이터");
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "0으로 초기화");
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "편집");
    menu_cnt++;

    status = view_input_combobox("강우량 데이터", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = aws_view_1min_data(LOGGING_RAIN_1MIN);
        break;
      case 2:
        status = aws_data_erase(FILE_RAIN);
        break;
      case 3:
        status = aws_edit_file(FILE_RAIN);
        break;
    }

    if (status == MENU_ABORT)
    {
      break ;
    }
  }

  return status;
}

int32_t aws_menu_data_sunshine(void)
{
  char buff[3][40];
  const char* menu[3];
  int choice, status;
  int menu_cnt;

  for (int i = 0; i < 3; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "1분 데이터");
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "0으로 초기화");
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "편집");
    menu_cnt++;

    status = view_input_combobox("일조 데이터", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = aws_view_1min_data(LOGGING_SUNSHINE_1MIN);
        break;
      case 2:
        status = aws_data_erase(FILE_SUNSHINE);
        break;
      case 3:
        status = aws_edit_file(FILE_SUNSHINE);
        break;
    }

    if (status == MENU_ABORT)
    {
      break; ;
    }
  }

  return status;
}

int32_t console_menu_data(void)
{
  char buff[4][40];
  const char* menu[4];
  int choice, status;
  int menu_cnt;

  for (int i = 0; i < 4; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "AWS");
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "강우량");
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "일조");
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "CSV 저장:%s",
             ITEM_LIST(config.aws_csv_save_active, enable_list_kor));
    menu_cnt++;

    status = view_input_combobox("데이터", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = aws_view_aws_data();
        break;
      case 2:
        status = aws_menu_data_rain();
        break;
      case 3:
        status = aws_menu_data_sunshine();
        break;
      case 4:
        choice = config.aws_csv_save_active;
        status = view_input_active("CSV 저장", &choice);
        if (status != MENU_OK)
          break;
        config.aws_csv_save_active = choice;
        WRITE_CFG(aws_csv_save_active);
        break;
    }

    if (status == MENU_ABORT)
    {
      break; 
    }
  }

  return status;
}
