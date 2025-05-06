

#include "console_aws_display.h"
#include "vt100_command.h"
#include "config_app.h"
#include "dev_io.h"

#include "task_tcpServer.h"
#include "task_direct.h"
#include "utile_time.h"
#include "aws_data.h"
#include "console_utile.h"
#include "app_charger.h"
#include "app_di.h"
#include "app_bsp.h"
#include "task_logging.h"
#include "cli_key_code.h"
#include "task_measure.h"
#include "dualport.h"
#include "task_logging.h"
#include "task_system.h"



#define AWS_MODE_MAX 3

#define DISP_WIDTH 26

const char *linkStatusList[] = {"-", "up", "down"};
const char *doorStatusList[] = {"닫힘", "열림"};
const char *generalStatusList[] = {"정상", "비정상"};


extern uint32_t g_debug_elased_time;
int32_t print_systemInfo(uint16_t row, uint16_t column)
{
  uint8_t line = row + 3;
  char buff[30];
  const char *message=NULL;

  snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d\r\n", Date_Time.Year, Date_Time.Month,
           Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);

  vt100_print_frame(row, column, "시스템", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "%s\r\n", buff);
  vt100_print_bar(line++, column, -DISP_WIDTH, "ID        :%d\r\n",0);
  vt100_print_bar(line++, column, -DISP_WIDTH, "문 상태   :%s\r\n",
                  ITEM_LIST(IS_DOOR_OPENED(), doorStatusList));

  if(get_logging_system()->status_group)
  {
    message = "오류";
  }
  else
  {
    message = "정상";
  }

  vt100_print_bar(line++, column, -DISP_WIDTH, "저장 기능 :%s\r\n",message);

  vt100_print_bar(line++, column, -DISP_WIDTH, "장비 전원 :%5.2f V\r\n", read_battery());
  vt100_print_bar(line++, column, -DISP_WIDTH, "장비 온도 :%5.2f C\r\n", read_temperature());

  if(get_config_app()->ac_use)
  {
  vt100_print_bar(line++, column, -DISP_WIDTH, "AC        :정상\r\n");
  }
  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return 4 + 2;
}

int32_t print_chargerInfo(uint16_t row, uint16_t column)
{
  char buff[10];

  uint8_t line = row + 3;
  uint8_t err;

  read_chargerStatus(buff, sizeof(buff));
  vt100_print_frame(row, column, "충전기", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "상태           :%s\r\n", buff);

  if (is_chargerValid())
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "  충전 전압(V) :%.2f\r\n",
                    read_solarVoltage1(&err));
    vt100_print_bar(line++, column, -DISP_WIDTH, "  충전 전류(A) :%.2f\r\n",
                    read_solarCurrrent1(&err));
    vt100_print_bar(line++, column, -DISP_WIDTH, "배터리 전압(V) :%.2f\r\n",
                    read_batteryVoltage1(&err));
    vt100_print_bar(line++, column, -DISP_WIDTH, " 부하1 전류(A) :%.2f\r\n",
                    read_loadCurrent1(&err));
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "  충전 전압(V) :--\r\n", 0);
    vt100_print_bar(line++, column, -DISP_WIDTH, "  충전 전류(A) :--\r\n", 0);
    vt100_print_bar(line++, column, -DISP_WIDTH, "배터리 전압(V) :--\r\n", 0);
    vt100_print_bar(line++, column, -DISP_WIDTH, " 부하1 전류(A) :--\r\n", 0);
  }
  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return 4 + 2;
}

int32_t print_rainInfo(uint16_t row, uint16_t column)
{
  char buff[30];

  uint8_t line = row + 3;

  make_comList(buff, sizeof(buff));
  vt100_print_frame(row, column, "강수량", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "전일:%6.1f\r\n",get_rainfall()->rainfall_yesterday);
  vt100_print_bar(line++, column, -DISP_WIDTH, "금일:%6.1f\r\n", get_rainfall()->rainfall_today);
  vt100_print_bar(line++, column, -DISP_WIDTH, "시간:%6.1f\r\n", get_rainfall()->rainfall_hourly);
  vt100_print_bar(line++, column, -DISP_WIDTH, "월간:%6.1f\r\n", get_rainfall()->rainfall_monthly);
  vt100_print_bar(line++, column, -DISP_WIDTH, "연간:%6.1f\r\n", get_rainfall()->rainfall_yearly);

  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line - (row);
}

int32_t print_ethInfo(uint16_t row, uint16_t column)
{
  char buff[30];

  uint8_t line = row + 3;

  make_comList(buff, sizeof(buff));
  vt100_print_frame(row, column, "이더넷", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "링크  :%s\r\n",
                  ITEM_LIST(get_direct_system()->link_status + 1, linkStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "송신  :%d\r\n", get_tcp_system()->tx_cnt);
  vt100_print_bar(line++, column, -DISP_WIDTH, "수신  :%d\r\n", get_tcp_system()->rx_cnt);

  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line - (row);
}

int32_t print_cdmaInfo(uint16_t row, uint16_t column)
{
  char buff[30];
  char num[20];
  uint8_t line = row + 3;
  int8_t rssi;

  make_comList(buff, sizeof(buff));
  vt100_print_frame(row, column, "CDMA", '+', '|', '-', DISP_WIDTH, WHITE);
  if (System.cdma_link_status == -1)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "링크    :-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "링크    :%s\r\n",
                    ITEM_LIST(System.cdma_link_status + 1, linkStatusList));
  }
  if (System.cdma_num[0] != '0')
  {
    num[0] = '-';
    num[1] = 0;
  }
  else
  {
    snprintf(num, sizeof(num), "%s", System.cdma_num);
  }
  vt100_print_bar(line++, column, -DISP_WIDTH, "전화번호:%s\r\n", num);
  if (System.cdma_rssi == -1)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "수신감도:-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "수신감도:%d\r\n", System.cdma_rssi);
  }

  vt100_print_bar(line++, column, -DISP_WIDTH, "송신    :%d\r\n", get_tcp_system()->tx_cnt);
  vt100_print_bar(line++, column, -DISP_WIDTH, "수신    :%d\r\n", get_tcp_system()->rx_cnt);
  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line - (row);
}

int32_t print_directInfo(uint16_t row, uint16_t column)
{
  char buff[30];
  char num[20];
  uint8_t line = row + 3;
  int8_t rssi;

  make_comList(buff, sizeof(buff));
  vt100_print_frame(row, column, "DIRECT", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "링크    :%s\r\n",
                  ITEM_LIST(get_direct_system()->link_status + 1, linkStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "송신    :%d\r\n", get_direct_system()->tx_cnt);
  vt100_print_bar(line++, column, -DISP_WIDTH, "수신    :%d\r\n", get_direct_system()->rx_cnt);
  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line - (row);
}

//[AWS = (관측값+100)/10, 관측값 = (x-1000)/10]
#define KMA_TO_TEMPERATURE(x) ((float)((x - 1000) / 10.0f))  
#define KMA_TO_GENERAL(x) ((float)(x / 10.0f))
#define KMA_TO_1000(x) ((float)((x - 1000) / 10.0f))
#define COL_WIDTH 15

int32_t print_awsRealLefinfo(uint16_t row, uint16_t column, eAWS_DATA_MIN_t min, void *arg)
{
  const char *aswTitleList[] = {"순간", "1분", "10분", "한시간"};
  char buff[50];
  uint8_t err;
  uint8_t line = row + 3;
  kma_data_ex_t *p_kma = NULL;
  uint32_t elapsed_time;

  p_kma = get_kma_data(min);


  elapsed_time = g_exec_250ms_time.elapsed_time + g_exec_1s_time.elapsed_time;

  snprintf(buff, sizeof(buff), "AWS %s %.2fms", aswTitleList[min], (float)elapsed_time / 1000.0f);

  vt100_print_frame(row, column, buff, '+', '|', '-', DISP_WIDTH, WHITE);

  if (p_kma->temperature.enable)
  {
    err = p_kma->temperature.err; 
    
    if(err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "기온",err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f C\r\n", COL_WIDTH, "기온",
                      KMA_TO_TEMPERATURE(p_kma->temperature.data));
    }
  }

  if (p_kma->wind_direction_avg.enable)
  {
    err = p_kma->wind_direction_avg.err; 
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "풍향",err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f 도\r\n", COL_WIDTH, "풍향",
                      KMA_TO_GENERAL(p_kma->wind_direction_avg.data));
    }
  }

  if (p_kma->wind_speed_avg.enable)
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "풍속",err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f m/s\r\n", COL_WIDTH, "풍속",
                      KMA_TO_GENERAL(p_kma->wind_speed_avg.data));
    }
  }

  if (p_kma->wind_direction_instant.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f 도\r\n", COL_WIDTH, "순간 풍향",
                    KMA_TO_GENERAL(p_kma->wind_direction_instant.data));
  }
  if (p_kma->wind_speed_instant.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f m/s\r\n", COL_WIDTH, "순간 풍속",
                    KMA_TO_GENERAL(p_kma->wind_speed_instant.data));
  }

  if (p_kma->precipitation.enable)
  {
    if (p_kma->precipitation.err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:error\r\n", COL_WIDTH, "강수량");
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f mm\r\n", COL_WIDTH, "강수량",
                      KMA_TO_GENERAL(p_kma->precipitation.data));
    }
  }

  if (p_kma->pressure.enable)
  {
    err = p_kma->pressure.err;
    
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "기압",err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f bar\r\n", COL_WIDTH, "기압",
                      KMA_TO_GENERAL(p_kma->pressure.data));
    }
  }

  if (p_kma->precipitation_presence.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5s\r\n", COL_WIDTH, "강수유무",
                    p_kma->precipitation_presence.data == 10 ? "유" : "무");
  }

  if (p_kma->snowfall.enable)
  {
    err = p_kma->snowfall.err;
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "적설",err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5d mm\r\n", COL_WIDTH, "적설",
                      (int)(p_kma->snowfall.data / 10.0f));
    }
  }

  if (p_kma->relative_humidity.enable)
  {
    err= p_kma->relative_humidity.err;
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "상대습도",err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f %%\r\n", COL_WIDTH, "상대습도",
                      KMA_TO_GENERAL(p_kma->relative_humidity.data));
    }
  }

  if (p_kma->solar_radiation.enable)
  {
    err = p_kma->solar_radiation.err;
    if(err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "일사",err);  
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f MJ/m2\r\n", COL_WIDTH, "일사",
                      p_kma->solar_radiation.data / 100.f); 
    }
  }

  if (p_kma->sunshine_duration.enable)
  {
    err = p_kma->sunshine_duration.err;
    if(err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "일조",err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5d s\r\n", COL_WIDTH, "일조",
                      (int)p_kma->sunshine_duration.data);
    }

  }

  if (p_kma->soil_temperature_5cm.enable)
  {
    err = p_kma->soil_temperature_5cm.err;

    if(err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "지중온도 5cm",
                      err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 5cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.data));
    }

  }

  if (p_kma->soil_temperature_10cm.enable)
  {
    err = p_kma->soil_temperature_10cm.err;
    
    if(err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "지중온도 10cm",
                      err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 10cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.data));
    }

  }
  if (p_kma->soil_temperature_20cm.enable)
  {
    err = p_kma->soil_temperature_20cm.err;

    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "지중온도 20cm",
                      err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 20cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.data));
    }
  }

  if (p_kma->soil_temperature_30cm.enable)
  {
    err = p_kma->soil_temperature_30cm.err;

    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "지중온도 30cm",
                      err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 30cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.data));
    }
  }

  if (p_kma->soil_temperature_50cm.enable)
  {
    err = p_kma->soil_temperature_50cm.err;
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "지중온도 50cm",
                      err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 50cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.data));
    }
  }

  if (p_kma->soil_temperature_1m.enable)
  {
    err = p_kma->soil_temperature_1m.err;
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "지중온도 1m",
                      err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 1m",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.data));
    }
  }
  if (p_kma->soil_temperature_1_5m.enable)
  {
    err = p_kma->soil_temperature_1_5m.err;
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "지중온도 1.5m",
                      err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 1.5m",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.data));
    }
  }

  if (p_kma->soil_temperature_3m.enable)
  {
    err = p_kma->soil_temperature_3m.err;
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "지중온도 3m",
                      err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 3m",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.data));
    }
  }
  if (p_kma->soil_temperature_5m.enable)
  {
    err = p_kma->soil_temperature_5m.err;
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:에러0x%02X\r\n", COL_WIDTH, "지중온도 5m",
                      err);
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 5m",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.data));
    }
  }

  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line;
}

int32_t aws_menu_display(p_shell_context_t ctx)
{
  keycode_t key;
  int32_t line = 0;
  uint8_t awsMode = 0;

  debug_printf(VT100_CLEAR_SCREEN);
  debug_printf(VT100_CURSOR_OFF);

  do
  {
    debug_printf(VT100_CURSOR_HOME);
    debug_printf("\r\n");
    print_systemInfo(1, 0);

    line = 0;
    if (get_config_app()->cdma_use)
    {
      line = print_cdmaInfo(1, 30);
    }
    if (get_config_app()->direct_use)
    {
      line += print_directInfo(1 + line, 30);
    }
    if (get_config_app()->eth_use)
    {
      line += print_ethInfo(1 + line, 30);
    }

    print_chargerInfo(20, 0);
    line = 0;
    line = print_awsRealLefinfo(1, 60, awsMode, NULL);

    print_rainInfo(1+line,60);

    key = get_key(500);

    if (key == KEY_CODE_RIGHT)
    {
      debug_printf(VT100_CLEAR_SCREEN);
      if (awsMode < AWS_MODE_MAX)
      {
        awsMode++;
      }
    }
    else if (key == KEY_CODE_LEFT)
    {
      debug_printf(VT100_CLEAR_SCREEN);
      if (awsMode > 0)
      {
        awsMode--;
      }
    }

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }
  } while (1);

  vt100_print(50, 0, "\r\n");
  debug_printf(VT100_CURSOR_ON);
  return 0;
}