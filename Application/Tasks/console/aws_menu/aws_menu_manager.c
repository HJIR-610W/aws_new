#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"
#include "util_time.h"
#include "update_fw.h"
#include "app_sensor.h"
#include "app_rs485.h"
#include "config_app.h"
#include "config_sensor.h"
#include "driver_485.h"
#include "app_dataLogging.h"
#include "cli_input.h"
#include "system_err.h"
#include "config_nvm.h"

#include "app_version.h"
#include "boot_version.h"
#include "app_flash.h"
#include "config_manager.h"
#include "Data\utile_data.h"
#include "app_logging.h"

int32_t menu_manage_version()
{
  char buff[30];

  uint8_t major, minor, fix, rel;
  DATE_TIME_BUF ct;

  get_app_version(&major, &minor, &fix, &rel);
  get_app_build(&ct);

  io_printf("App:%d.%d.%d.%d\r\n", major, minor, fix, rel);
  make_timeToStr(&ct, buff, sizeof(buff));
  io_printf("App build:%s\r\n", buff);

  get_boot_version(&major, &minor, &fix, &rel);
  get_boot_build(&ct);

  io_printf("Boot:%d.%d.%d.%d\r\n", major, minor, fix, rel);
  make_timeToStr(&ct, buff, sizeof(buff));
  io_printf("Boot build:%s\r\n", buff);
  return MENU_OK;
}

int32_t download_file(int32_t (*save_file)(char *path, uint32_t offset, uint8_t *data,
                                           uint32_t dataLen),
                      char *path, uint32_t offset, uint32_t *len, uint32_t limit);

int32_t save_file(char *path, uint32_t offset, uint8_t *data, uint32_t dataLen)
{
  flash_write(offset, data, dataLen);

  return 0;
}

int32_t menu_manage_update()
{
  char buff[20];
  uint32_t len;

  io_printf("10초뒤에 파일을 전송해주세요\r\n");
  osDelay(10000);

  if (download_file(save_file, buff, 0, &len, 512 * 1024) == 0)
  {
    io_printf("파일 크기:%d\r\n", len);
  }
  else
  {
    io_printf("파일 수신 오류\r\n");
  }

  return 0;
}
int32_t menu_manage_device_reset()
{
  reset_system("console reset");
  return MENU_OK;
}

void config_hj_reset(void)
{
  adc_config_t *adc_config;
  hjtemp_config_t *hjtemp_cfg;
  hjhumi_config_t *hjhumi_cfg;
  hjwindspeed_config_t *hjwind_cfg;
  hjwindDirection_config_t *hjwindDir_cfg;

  hjsnow_config_t *hjsnow_cfg;
  uint8_t single_channel = 0;

  config_app_reset();

  // config 중 센서 설정정보만 화진에 맞게 설정한다
  config_sensor_reset();

  // 온도 센서[화진 온도 9600]
  config.sensor[A1_TEMPERATURE].type = S_T_TEMPERATURE_HJ;
  sensor_add(&config.sensor[A1_TEMPERATURE]);
  hjtemp_cfg = get_sensor_config(&config.sensor[A1_TEMPERATURE]);
  hjtemp_cfg->physical_layer = ePHYSICAL_RS485;
  hjtemp_cfg->rs485_port = eAPP_RS485_D;
  hjtemp_cfg->modbus_id = 1;
  // 습도 센서[화진 습도 9600]
  config.sensor[A10_RELATIVE_HUMIDITY].type = S_T_HUMINITY_HJ;
  sensor_add(&config.sensor[A10_RELATIVE_HUMIDITY]);
  hjhumi_cfg = get_sensor_config(&config.sensor[A10_RELATIVE_HUMIDITY]);
  hjhumi_cfg->physical_layer = ePHYSICAL_RS485;
  hjhumi_cfg->rs485_port = eAPP_RS485_D;
  hjhumi_cfg->modbus_id = 1;

  // 풍향[화진 RS485 풍향 19200]
  config.sensor[A2_WIND_DIRECTION].type = S_T_WIND_DIRECTION_HJ_485;
  sensor_add(&config.sensor[A2_WIND_DIRECTION]);
  hjwindDir_cfg = get_sensor_config(&config.sensor[A2_WIND_DIRECTION]);
  hjwindDir_cfg->rs485_port = RS485_A;

  // 풍속[화진 RS485 풍속 19200]
  config.sensor[A3_WIND_SPEED].type = S_T_WIND_SPEED_HJ_485;
  sensor_add(&config.sensor[A3_WIND_SPEED]);
  hjwind_cfg = get_sensor_config(&config.sensor[A3_WIND_SPEED]);
  hjwind_cfg->rs485_port = RS485_A;
  hjwind_cfg->full = 3200;
  hjwind_cfg->offset = 0;

  // 강우감지[화진 접점]
  config.sensor[A8_RAIN_PRESENT].type = S_T_RAIN_PRESENT_DI;

  // 강수량[리드형]
  config.sensor[A6_RAINFALL_DOT5_1MM].type = S_T_RAIN_REED_1MM;

  // 적설[화진 RS485 19200]
  config.sensor[A9_SNOW_DEPTH].type = S_T_SNOW_HJ;
  sensor_add(&config.sensor[A9_SNOW_DEPTH]);
  hjsnow_cfg = get_sensor_config(&config.sensor[A9_SNOW_DEPTH]);
  hjsnow_cfg->physical_layer = ePHYSICAL_RS232;
  hjsnow_cfg->port = eRS232_HART_D;

  // 기압[RM YOUNG]
  config.sensor[A7_PRESSURE].type = S_T_ADC;
  sensor_add(&config.sensor[A7_PRESSURE]);
  adc_config = get_sensor_config(&config.sensor[A7_PRESSURE]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 200000;
  adc_config->lowScale = 0;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // 일사 CMP3 0~1.0VDC
  config.sensor[B1_SOLAR_RADIATION].type = S_T_ADC;
  sensor_add(&config.sensor[B1_SOLAR_RADIATION]);
  adc_config = get_sensor_config(&config.sensor[B1_SOLAR_RADIATION]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 5000;  // 5v
  adc_config->lowScale = 0;      // 0v
  adc_config->scale = 1000;
  adc_config->outMaxV = 5000;
  adc_config->outMinV = 0;

  // 일조 CSD3 센서 출력 : 120 w/m2 이상일 때 1 VDC, 이하일 때 0 VDC
  // 센서값 자체를 전압으로 받는다
  config.sensor[B2_SUNSHINE_DURATION].type = S_T_ADC;
  sensor_add(&config.sensor[B2_SUNSHINE_DURATION]);
  adc_config = get_sensor_config(&config.sensor[B2_SUNSHINE_DURATION]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 5000;
  adc_config->lowScale = 0;
  adc_config->scale = 1000;
  adc_config->outMaxV = 5000;
  adc_config->outMinV = 0;

  // 지중온도 5cm
  config.sensor[B5_SOIL_TEMPERATURE_5CM].type = S_T_ADC;
  sensor_add(&config.sensor[B5_SOIL_TEMPERATURE_5CM]);
  adc_config = get_sensor_config(&config.sensor[B5_SOIL_TEMPERATURE_5CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // 지중온도 10cm
  config.sensor[B6_SOIL_TEMPERATURE_10CM].type = S_T_ADC;
  sensor_add(&config.sensor[B6_SOIL_TEMPERATURE_10CM]);
  adc_config = get_sensor_config(&config.sensor[B6_SOIL_TEMPERATURE_10CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // 지중온도 20cm
  config.sensor[B7_SOIL_TEMPERATURE_20CM].type = S_T_ADC;
  sensor_add(&config.sensor[B7_SOIL_TEMPERATURE_20CM]);
  adc_config = get_sensor_config(&config.sensor[B7_SOIL_TEMPERATURE_20CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // 지중온도 30cm
  config.sensor[B8_SOIL_TEMPERATURE_30CM].type = S_T_ADC;
  sensor_add(&config.sensor[B8_SOIL_TEMPERATURE_30CM]);
  adc_config = get_sensor_config(&config.sensor[B8_SOIL_TEMPERATURE_30CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // 지중온도 50cm
  config.sensor[B9_SOIL_TEMPERATURE_50CM].type = S_T_ADC;
  sensor_add(&config.sensor[B9_SOIL_TEMPERATURE_50CM]);
  adc_config = get_sensor_config(&config.sensor[B9_SOIL_TEMPERATURE_50CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // 지중온도 1m
  config.sensor[B10_SOIL_TEMPERATURE_100CM].type = S_T_ADC;
  sensor_add(&config.sensor[B10_SOIL_TEMPERATURE_100CM]);
  adc_config = get_sensor_config(&config.sensor[B10_SOIL_TEMPERATURE_100CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // 지중온도 1.5m
  config.sensor[B11_SOIL_TEMPERATURE_150CM].type = S_T_ADC;
  sensor_add(&config.sensor[B11_SOIL_TEMPERATURE_150CM]);
  adc_config = get_sensor_config(&config.sensor[B11_SOIL_TEMPERATURE_150CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  save_config_app();
  save_config_sensor();
}

#define AWS_MANAGER_MENU_WITDH 30
int32_t menu_manage_config_backup()
{
  int status;
  int choice;
  int ok;

   char *menu[] = {"백업", "복구"};

  while (1)
  {
    status = choice_menu(AWS_MANAGER_MENU_WITDH, "설정 백업/복구", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      break; 

    switch (choice)
    {
      case 1:
        backup_config();
        io_printf("SD카드에 백업되었습니다\r\n");
        break;
      case 2:
        status = confirm_continue("SD카드에서 설정값을 불러옵니다",&ok);
        if(status!=MENU_OK)
        break;
        if(ok)
          restore_config();
        
        break;
    }
    if(status ==MENU_ABORT)
    break;
  }
  return status;
}

int32_t menu_manage_sentor_edit()
{

  int choice;
  int status;
   char *menu[] = {"우량 자료 편집",
                   "일조 자료 편집",
                   "우량 자료 확인",
                   "일조 자료 확인"};

  char start_time[30];
  char end_time[30];  // 2025-01-01 00:00:00
  int32_t value;
  const char *filename;
  int32_t ret;
    int ok;
  while (1)
  {
    status = choice_menu(24, "데이터 편집", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
      case 2:
        if(confirm_continue("해당년도 자료 모두 0으로 초기화 할까요?",&ok)==MENU_OK && ok ==1)
        {

            
          snprintf(start_time, sizeof(start_time), "%04d-01-01 00:01:00", Date_Time.Year);
          snprintf(end_time, sizeof(end_time), "%04d-01-01 00:00:00", Date_Time.Year + 1);
          if (choice == 1)
          {
            filename = "RAIN_01.rcd";
            ret = write_bulk_data_range(filename, start_time, end_time, value);
            if (ret < 0)
            {
              io_printf("에러 발생 코드:%d\r\n", ret);
            }
          }
          else
          {
            filename = "SUNSHINE_01.rcd";
            ret = write_bulk_data_range(filename, start_time, end_time, value);
            if (ret < 0)
            {
              io_printf("에러 발생 코드:%d\r\n", ret);
            }
          }
        }
        else
        {


        io_printf("시작시간입력(예:2025-01-01 00:01:00)\r\n");
        io_printf(">>");
        cli_scanf_s("%[^\n]", start_time, (unsigned)_countof(start_time));
        io_printf("종료시간입력(예:2025-01-01 00:01:00)\r\n");
        io_printf(">>");
        cli_scanf_s("%[^\n]", end_time, (unsigned)_countof(end_time));
        io_printf("갑 입력\r\n");
        io_printf(">>");
        cli_scanf_s("%d", &value);
        ;
        if (choice == 1)
        {
          filename = "RAIN_01.rcd";
        }
        else if (choice == 2)
        {
          filename = "SUNSHINE_01.rcd";
        }

        status = confirm_continue("계속 진행하겠니까?",&ok);
        if(status!=MENU_OK)
        break;
        if(ok)
        {
          io_printf("범위를 넓게 하면 편집에 수십초가 소요될 수 있습니다\r\n");
          ret = write_bulk_data_range(filename, start_time, end_time, value);
          if (ret < 0)
          {
            io_printf("에러 발생 코드:%d\r\n", ret);
          }
          io_printf("OK\r\n");
        }
      }
        break;

      case 3:
      case 4:
      {
        int year;
        int month;
        int day;
        int hour;
        int min;
        int read_cnt;
        io_printf("시작시간입력(예:2025-01-01 00:01)\r\n");
        io_printf(">>");
        cli_scanf_s("%04d-%02d-%02d %02d:%02d", &year, &month, &day, &hour, &min);
        io_printf("읽을 갯수 입력\r\n");
        io_printf(">>");
        cli_scanf_s("%d", &read_cnt);
        DATE_TIME_BUF ct;
        ct.Year = year;
        ct.Month = month;
        ct.Day = day;
        ct.Hour = hour;
        ct.Min = min;
        ct.Sec = 0;
        uint32_t start_time = time_cvt_timestamp(&ct);

        for (int i = 0; i < read_cnt; i++)
        {
          uint16_t data;
          uint8_t type;
          type = choice == 3 ? LOGGING_RAIN_1MIN : LOGGING_SUNSHINE_1MIN;
          read_sensorDataMulti(&ct, sizeof(uint16_t), 1, type, 1, (uint8_t *)&data, sizeof(data));
          io_printf("%04d-%02d-%02d %02d:%02d:00 %5d\r\n", ct.Year, ct.Month, ct.Day, ct.Hour,
                    ct.Min, data);
          start_time += 60;
          time_cvt_secTotime(start_time, &ct);
        }
      }
      
    }
    if(status == MENU_ABORT)
    break;
  }

  return status;
}


int32_t menu_manage_update_fw()
{
  int status;
  int ok;

  while(1)
  {
    status  = confirm_continue("펌웨어 업데이트를 진행할까요?",&ok);
    if(status != MENU_OK)
    break;
    
    if(ok==0)
    {
      status = MENU_BACK;
      break;
    }
    
      if (check_firmware(UPDATE_LOCAL) == 0)
      {
        io_printf("장비가 리셋되면서 업데이트가 진행됩니다\r\n");
        io_printf("상태 LED가 점멸됩니다\r\n");

        set_magic_value(MAGIC_UPDATE_FW_LACAL);
        reset_system("USER update");
      }


  }

  return status;
}
int32_t menu_manage_log_reset(void)
{
  int status;
  int log_cnt;

  io_printf("현재 로그 카운트:%d\r\n", get_config_nvm()->log_q_cnt );

  status = input_decimal_prompt("로그 카운트 입력해주세요", &log_cnt, 0, LOG_COUNT_MAX);
  if(status == MENU_OK)
  {
    nvm_set_log_cnt(log_cnt);
  }

  return status;
}
  int aws_manager_config(void)
  {
    int choice, status;
    int ok;
    char *menu[] = {"AWS 화진 기본 설정", "공장 초기화", "설정 백업", "우량,일조 자료 초기화",
                    "로그 카운트 초기화"};

    while (1)
    {
      status = choice_menu(24, "DATA", menu, _countof(menu), &choice);
      if (status != MENU_OK)
        return status;

      switch (choice)
      {
        case 1:
          status = confirm_continue("센서 구성을 화진 기본값으로 초기화합니다", &ok);
          if (status != MENU_OK)
            break;

          if (ok)
          {
            config_hj_reset();
            io_printf("초기화 되었습니다");
          }
          break;
        case 2:
          status = confirm_continue("설정값을 공장초기화합니다", &ok);

          if (status != MENU_OK)
            break;
          if (ok)
          {
            config_app_reset();
            save_config_app();
            config_sensor_reset();
            save_config_sensor();
            io_printf("공장 초기화 되었습니다\r\n");
          }
          break;
        case 3:
          status = menu_manage_config_backup();
          break;
        case 4:
          status = menu_manage_sentor_edit();
          break;
        case 5:
          status = menu_manage_log_reset();
          break;
      }

      if (status != MENU_OK)
      {
        break;
      }
    }

    return status;
  }

  int aws_menu_manager(void)
  {
    int choice, status;
    char *menu[] = {"버전", "장비리셋", "설정 변경", "펌웨어 업데이트"};

    while (1)
    {
      status = choice_menu(24, "설정", menu, _countof(menu), &choice);
      if (status != MENU_OK)
        return status;

      switch (choice)
      {
        case 1:
          status = menu_manage_version();
          break;
        case 2:
          status = menu_manage_device_reset();
          break;
        case 3:
          status = aws_manager_config();
          break;
        case 4:
          status = menu_manage_update_fw();
          break;
      }
      if (status != MENU_OK)
      {
        break;
      }
    }

    return status;
  }