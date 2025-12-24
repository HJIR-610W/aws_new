#include "app_dataLogging.h"

#include "app_logging.h"
#include "drv_rs485.h"
#include "app_sensor.h"
#include "app_version.h"
#include "boot_version.h"
 
#include "cmsis_os2.h"
#include "config_app.h"
#include "config_manager.h"
#include "config_nvm.h"
#include "config_sensor.h"
#include "console_define.h"
#include "console_utile.h"
#include "debug_io.h"
#include "drv_rs232.h"
#include "drv_rs485.h"
#include "drv_flash.h"
#include "logging\utile_data.h"
#include "system_err.h"
#include "update_fw.h"
#include "util_memory.h"
#include "util_time.h"
#include "const_string.h"

int32_t menu_manage_version()
{
  char buff[30];

  uint8_t major, minor, fix, rel;
  DATE_TIME_BUF ct;

  get_app_version(&major, &minor, &fix, &rel);
  get_app_build(&ct);

  debug_printf("App:%d.%d.%d.%d\r\n", major, minor, fix, rel);
  make_time_to_string(&ct, buff, sizeof(buff));
  debug_printf("App build:%s\r\n", buff);

  get_boot_version(&major, &minor, &fix, &rel);
  get_boot_build(&ct);

  debug_printf("Boot:%d.%d.%d.%d\r\n", major, minor, fix, rel);
  make_time_to_string(&ct, buff, sizeof(buff));
  debug_printf("Boot build:%s\r\n", buff);
  return MENU_OK;
}

int32_t download_file(int32_t (*save_file)(char *path, uint32_t offset, uint8_t *data,
                                           uint32_t dataLen),
                      char *path, uint32_t offset, uint32_t *len, uint32_t limit);

int32_t save_file(char *path, uint32_t offset, uint8_t *data, uint32_t dataLen)
{
  drv_flash_write(offset, data, dataLen);

  return 0;
}

int32_t menu_manage_update()
{
  char buff[20];
  uint32_t len;

  debug_printf("10초뒤에 파일을 전송해주세요\r\n");
  osDelay(10000);

  if (download_file(save_file, buff, 0, &len, 512 * 1024) == 0)
  {
    debug_printf("파일 크기:%d\r\n", len);
  }
  else
  {
    debug_printf("파일 수신 오류\r\n");
  }

  return 0;
}
int32_t menu_manage_device_reset()
{
  reset_system("console reset");
  return MENU_OK;
}

int32_t menu_manage_lcd_off_time()
{
  int status;
  int lcd_off_time;

  lcd_off_time = (int32_t)get_config_app()->lcd_off_time_index;
  status = view_input_combobox("LCD Off Time (분)", lcd_off_time_list_eng,_countof(lcd_off_time_list_eng),&lcd_off_time );
  if (status == MENU_OK)
  {
    get_config_app()->lcd_off_time_index = (eLCD_OFF_TIME_t)(lcd_off_time-1);
    WRITE_CFG(lcd_off_time_index);
  }

  return status;
}


void config_hj_reset(void)
{
  adc_config_t *adc_config;
  temp_hj_config_t *hjtemp_cfg;
  humi_hj_config_t *hjhumi_cfg;
  wind_speed_hj_config_t *hj_wind_speed;
  wind_direction_hj_config_t *hj_wind_direction;
  snow_hj_config_t *hjsnow_cfg;
  rain_present_config_t *hjrain_det_cfg;
  barometer_rmyoung_61302v_rs232_config_t *p_barometer;
  rainfall_reed_t *p_rain_reed;
solar_duration_csd3_t *p_solar_duration;
solar_r_ott_smp3_config_t *p_smp3;

      uint8_t single_channel = 0;

  config_app_sensor_reset();

  // config 중 센서 설정정보만 화진에 맞게 설정한다
  config_sensor_reset();

  // 온도 센서[화진 온도 9600]
  config.sensor[A1_TEMPERATURE].model = S_T_TEMPERATURE_HJ;

  hjtemp_cfg = get_sensor_config(A1_TEMPERATURE,S_T_TEMPERATURE_HJ);
  hjtemp_cfg->physical_layer = ePHYSICAL_RS485;
  hjtemp_cfg->rs485_port = eAPP_RS485_RS232_B;
  hjtemp_cfg->modbus_id = 1;
  // 습도 센서[화진 습도 9600]
  config.sensor[A10_RELATIVE_HUMIDITY].model = S_T_HUMINITY_HJ;

  hjhumi_cfg = get_sensor_config(A10_RELATIVE_HUMIDITY,S_T_HUMINITY_HJ);
  hjhumi_cfg->physical_layer = ePHYSICAL_RS485;
  hjhumi_cfg->rs485_port = eAPP_RS485_RS232_B;
  hjhumi_cfg->modbus_id = 1;

  // 풍향[화진 RS485 풍향 19200 modbus]
  config.sensor[A2_WIND_DIRECTION].model = S_T_WIND_DIRECTION_HJ_MODBUS;

  hj_wind_direction =get_sensor_config(A2_WIND_DIRECTION,S_T_WIND_DIRECTION_HJ_MODBUS);
  hj_wind_direction->rs485_port = eAPP_RS485_C;
  hj_wind_direction->modbus_id = 2;

  // 풍속[화진 RS485 풍속 19200 modbus]
  config.sensor[A3_WIND_SPEED].model = S_T_WIND_SPEED_HJ_MODBUS;

  hj_wind_speed = get_sensor_config(A3_WIND_SPEED,S_T_WIND_SPEED_HJ_MODBUS);
  hj_wind_speed->rs485_port = eAPP_RS485_C;
  hj_wind_speed->modbus_id = 1;

  // 강우감지[화진 접점]
  config.sensor[A8_RAIN_PRESENT].model = S_T_RAIN_PRESENT_DI;

  hjrain_det_cfg = get_sensor_config(A8_RAIN_PRESENT,S_T_RAIN_PRESENT_DI);
  hjrain_det_cfg->off_delay_sec = 300;

  // 강수량[리드형]
  config.sensor[A6_RAINFALL_DOT5_1MM].model = S_T_RAIN_REED;
  p_rain_reed = get_sensor_config(A6_RAINFALL_DOT5_1MM,S_T_RAIN_REED);
  p_rain_reed->mm = eRAIN_05MM;
  

  // 적설[화진 RS485 19200]
  config.sensor[A9_SNOW_DEPTH].model = S_T_SNOW_HJ;

  hjsnow_cfg = get_sensor_config(A9_SNOW_DEPTH,S_T_SNOW_HJ);
  hjsnow_cfg->physical_layer = ePHYSICAL_RS232;
  hjsnow_cfg->rs232_port = eRS232_C;

  // 기압[RM YOUNG]
  config.sensor[A7_PRESSURE].model = S_T_BARO_RMYOUNG_61302V_RS232;

  p_barometer =get_sensor_config(A7_PRESSURE,S_T_BARO_RMYOUNG_61302V_RS232);
  p_barometer->rs232_port = eRS232_RS485_A;

  // 일사 CMP3 0~1.0VDC
  config.sensor[B1_SOLAR_RADIATION].model = S_T_SOLAR_RADIATION_OTT_SMP3;

  p_smp3 = get_sensor_config(B1_SOLAR_RADIATION,S_T_SOLAR_RADIATION_OTT_SMP3);
  p_smp3->rs485_port = eAPP_RS485_RS232_B;
  p_smp3->modbus_id = 2;


  // 일조 CSD3 센서 출력 : 120 w/m2 이상일 때 1 VDC, 이하일 때 0 VDC
  // 센서값 자체를 전압으로 받는다
  config.sensor[B2_SUNSHINE_DURATION].model = S_T_SOLAR_DURATION_CSD3;

  p_solar_duration = get_sensor_config(B2_SUNSHINE_DURATION,S_T_ADC);
  p_solar_duration->adc_channel = ADC_SUNSHINE_CSD3;//2


  // 지중온도 5cm
  config.sensor[B5_SOIL_TEMPERATURE_5CM].model = S_T_ADC;

  adc_config =  get_sensor_config(B5_SOIL_TEMPERATURE_5CM,S_T_ADC);
  adc_config->single_channel = ADC_SOIL5CM;
  adc_config->mode = eSINGLE_ADC;
  adc_config->high_scale = 60;
  adc_config->low_scale = -40;
  adc_config->scale = 1;
  adc_config->out_max_mv = 5000;
  adc_config->out_min_mv = 0;

  // 지중온도 10cm
  config.sensor[B6_SOIL_TEMPERATURE_10CM].model = S_T_ADC;

  adc_config =  get_sensor_config(B6_SOIL_TEMPERATURE_10CM,S_T_ADC);
  adc_config->single_channel = ADC_SOIL10CM;
  adc_config->mode = eSINGLE_ADC;
  adc_config->high_scale = 60;
  adc_config->low_scale = -40;
  adc_config->scale = 1;
  adc_config->out_max_mv = 5000;
  adc_config->out_min_mv = 0;


  // 지중온도 20cm
  config.sensor[B7_SOIL_TEMPERATURE_20CM].model = S_T_ADC;

  adc_config =  get_sensor_config(B7_SOIL_TEMPERATURE_20CM,S_T_ADC);
  adc_config->single_channel = ADC_SOIL20CM;
  adc_config->mode = eSINGLE_ADC;
  adc_config->high_scale = 60;
  adc_config->low_scale = -40;
  adc_config->scale = 1;
  adc_config->out_max_mv = 5000;
  adc_config->out_min_mv = 0;


  // 지중온도 30cm
  config.sensor[B8_SOIL_TEMPERATURE_30CM].model = S_T_ADC;

  adc_config = get_sensor_config(B8_SOIL_TEMPERATURE_30CM,S_T_ADC);
  adc_config->single_channel = ADC_SOIL30CM;
  adc_config->mode = eSINGLE_ADC;
  adc_config->high_scale = 60;
  adc_config->low_scale = -40;
  adc_config->scale = 1;
  adc_config->out_max_mv = 5000;
  adc_config->out_min_mv = 0;


  // 지중온도 50cm
  config.sensor[B9_SOIL_TEMPERATURE_50CM].model = S_T_ADC;

  adc_config = get_sensor_config(B9_SOIL_TEMPERATURE_50CM,S_T_ADC);
  adc_config->single_channel = ADC_SOIL50CM;
  adc_config->mode = eSINGLE_ADC;
  adc_config->high_scale = 60;
  adc_config->low_scale = -40;
  adc_config->scale = 1;
  adc_config->out_max_mv = 5000;
  adc_config->out_min_mv = 0;


  // 지중온도 1m
  config.sensor[B10_SOIL_TEMPERATURE_100CM].model = S_T_ADC;

  adc_config = get_sensor_config(B10_SOIL_TEMPERATURE_100CM,S_T_ADC);
  adc_config->single_channel = ADC_SOIL1M;
  adc_config->mode = eSINGLE_ADC;
  adc_config->high_scale = 60;
  adc_config->low_scale = -40;
  adc_config->scale = 1;
  adc_config->out_max_mv = 5000;
  adc_config->out_min_mv = 0;


  // 지중온도 1.5m
  config.sensor[B11_SOIL_TEMPERATURE_150CM].model = S_T_ADC;

  adc_config =get_sensor_config(B11_SOIL_TEMPERATURE_150CM,S_T_ADC);
  adc_config->single_channel = ADC_SOIL1_5M;
  adc_config->mode = eSINGLE_ADC;
  adc_config->high_scale = 60;
  adc_config->low_scale = -40;
  adc_config->scale = 1;
  adc_config->out_max_mv = 5000;
  adc_config->out_min_mv = 0;


  // 지중온도 3m
  config.sensor[B12_SOIL_TEMPERATURE_300CM].model = S_T_ADC;

  adc_config = get_sensor_config(B12_SOIL_TEMPERATURE_300CM,S_T_ADC);
  adc_config->single_channel = ADC_SOIL3M;
  adc_config->mode = eSINGLE_ADC;
  adc_config->high_scale = 60;
  adc_config->low_scale = -40;
  adc_config->scale = 1;
  adc_config->out_max_mv = 5000;
  adc_config->out_min_mv = 0;


  // 지중온도 5m
  config.sensor[B13_SOIL_TEMPERATURE_500CM].model = S_T_ADC;

  adc_config = get_sensor_config(B13_SOIL_TEMPERATURE_500CM,S_T_ADC);
  adc_config->single_channel = ADC_SOIL5M;
  adc_config->mode = eSINGLE_ADC;
  adc_config->high_scale = 60;
  adc_config->low_scale = -40;
  adc_config->scale = 1;
  adc_config->out_max_mv = 5000;
  adc_config->out_min_mv = 0;

  save_config_app();
  save_config_sensor();
}

#define AWS_MANAGER_MENU_WITDH 30
int32_t menu_manage_config_backup()
{
  int status;
  int choice=0;
  int ok;

   const char *menu[] = {"백업", "복구"};

  while (1)
  {
    status = view_input_combobox( "설정 백업/복구", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      break; 

    switch (choice)
    {
      case 1:
        backup_config();
        debug_printf("SD카드에 백업되었습니다\r\n");
        break;
      case 2:
        status = view_confirm_continue("SD카드에서 설정값을 불러옵니다",&ok);
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
  int year,month,day,hour,min,sec;
  int choice=0;
  int status;
   const char *menu[] = {"우량 자료 편집",
                   "일조 자료 편집",
                   "우량 자료 확인",
                   "일조 자료 확인"};
    DATE_TIME_BUF start_time;
    DATE_TIME_BUF end_time;


  int32_t value=0;
  const char *filename="";
  int32_t ret;
    int ok;
  while (1)
  {
    status = view_input_combobox( "데이터 편집", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
      case 2:
        if(view_confirm_continue("해당년도 자료 모두 0으로 초기화 할까요?",&ok)==MENU_OK && ok ==1)
        {
          start_time.Year = Date_Time.Year;
          start_time.Month =1;
          start_time.Day = 1;
          start_time.Hour = 0;
          start_time.Min  = 1;
          start_time.Sec = 0;

          end_time.Year = Date_Time.Year+1;
          end_time.Month = 1;
          end_time.Day = 1;
          end_time.Hour = 0;
          end_time.Min = 1;
          end_time.Sec = 0;


          if (choice == 1)
          {
            filename = "RAIN_01.rcd";
            ret = write_bulk_data_range(filename, &start_time, &end_time, value);
            if (ret < 0)
            {
              debug_printf("에러 발생 코드:%d\r\n", ret);
            }
          }
          else
          {
            filename = "SUNSHINE_01.rcd";
            ret = write_bulk_data_range(filename, &start_time, &end_time, value);
            if (ret < 0)
            {
              debug_printf("에러 발생 코드:%d\r\n", ret);
            }
          }
        }
        else
        {


        debug_printf("시작시간입력(예:2025-01-01 00:01:00)\r\n");
        debug_printf(">>");
        debug_scanf_s("%04d-%02d-%02d %02d:%02d:%02d", &year,&month,&day,&hour,&min,&sec);

        start_time.Year = year;
        start_time.Month = month;
        start_time.Day = day;
        start_time.Hour = hour;
        start_time.Min = min;
        start_time.Sec = sec;

        debug_printf("종료시간입력(예:2025-01-01 00:01:00)\r\n");
        debug_printf(">>");
        debug_scanf_s("%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec);
        end_time.Year = year;
        end_time.Month = month;
        end_time.Day = day;
        end_time.Hour = hour;
        end_time.Min = min;
        end_time.Sec = sec;

        debug_printf("갑 입력\r\n");
        debug_printf(">>");
        debug_scanf_s("%d", &value);
        ;
        if (choice == 1)
        {
          filename = "RAIN_01.rcd";
        }
        else if (choice == 2)
        {
          filename = "SUNSHINE_01.rcd";
        }

        status = view_confirm_continue("계속 진행하겠니까?",&ok);
        if(status!=MENU_OK)
        break;
        if(ok)
        {
          debug_printf("범위를 넓게 하면 편집에 수십초가 소요될 수 있습니다\r\n");
          ret = write_bulk_data_range(filename, &start_time, &end_time, value);
          if (ret < 0)
          {
            debug_printf("에러 발생 코드:%d\r\n", ret);
          }
          debug_printf("OK\r\n");
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
        debug_printf("시작시간입력(예:2025-01-01 00:01)\r\n");
        debug_printf(">>");
        debug_scanf_s("%04d-%02d-%02d %02d:%02d", &year, &month, &day, &hour, &min);
        debug_printf("읽을 갯수 입력\r\n");
        debug_printf(">>");
        debug_scanf_s("%d", &read_cnt);
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
          debug_printf("%04d-%02d-%02d %02d:%02d:00 %5d\r\n", ct.Year, ct.Month, ct.Day, ct.Hour,
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
    status  = view_confirm_continue("펌웨어 업데이트를 진행할까요?",&ok);
    if(status != MENU_OK)
    break;
    
    if(ok==0)
    {
      status = MENU_BACK;
      break;
    }
    
      if (check_firmware(UPDATE_LOCAL) == 0)
      {
        debug_printf("장비가 리셋되면서 업데이트가 진행됩니다\r\n");
        debug_printf("상태 LED가 점멸됩니다\r\n");

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

  debug_printf("현재 로그 카운트:%d\r\n", get_config_nvm()->log_q_cnt );

  status = view_input_decimal("로그 카운트 입력해주세요", &log_cnt, 0, LOG_COUNT_MAX);
  if(status == MENU_OK)
  {
    nvm_set_log_cnt(log_cnt);
  }

  return status;
}
  int aws_manager_config(void)
  {
    int choice=0, status;
    int ok;
    const char *menu[] = {"AWS 화진 기본 설정", "공장 초기화", "설정 백업", "우량,일조 자료 초기화",
                    "로그 카운트 초기화"};

    while (1)
    {
      status = view_input_combobox( "DATA", menu, _countof(menu), &choice);
      if (status != MENU_OK)
        return status;

      switch (choice)
      {
        case 1:
          status = view_confirm_continue("센서 구성을 화진 기본값으로 초기화합니다", &ok);
          if (status != MENU_OK)
            break;

          if (ok)
          {
            config_hj_reset();
            debug_printf("초기화 되었습니다");
          }
          break;
        case 2:
          status = view_confirm_continue("설정값을 공장초기화합니다", &ok);

          if (status != MENU_OK)
            break;
          if (ok)
          {
            config_app_reset();
            save_config_app();
            config_sensor_reset();
            save_config_sensor();
            debug_printf("공장 초기화 되었습니다\r\n");
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
    int choice = 0;
    int status;
    const char *menu[] = {"버전", "설정 변경", "펌웨어 업데이트","LCD Off Time", "장비리셋",};

    while (1)
    {

      status = view_input_combobox( "설정", menu, _countof(menu), &choice);
      if (status != MENU_OK)
        break;

      switch (choice)
      {
        case 1:
          status = menu_manage_version();
          break;
        case 2:
          status = aws_manager_config();
          break;
        case 3:
          status = menu_manage_update_fw();
          break;
        case 4:
          status = menu_manage_lcd_off_time();
          break;
        case 5:
          status = menu_manage_device_reset();
          break;
        }
      if (status != MENU_OK)
      {
        break;
      }
    }

    return status;
  }