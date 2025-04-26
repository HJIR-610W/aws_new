

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

  snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d\r\n", Date_Time.Year, Date_Time.Month,
           Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);

  vt100_print_frame(row, column, "시스템", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "%s\r\n", buff);
  vt100_print_bar(line++, column, -DISP_WIDTH, "문 상태   :%s\r\n",
                  ITEM_LIST(IS_DOOR_OPENED(), doorStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "저장 기능 :%s\r\n",
                  ITEM_LIST(IS_DATA_ERR(), generalStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "장비 전원 :%5.2f V\r\n", read_battery());
  vt100_print_bar(line++, column, -DISP_WIDTH, "장비 온도 :%5.2f C\r\n", read_temperature());

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

int32_t print_awsRealLefinfo(uint16_t row, uint16_t column, uint8_t mode, void *arg)
{
  char buff[50];
  uint8_t line = row + 3;
  const char *aswTitleList[] = {"RAW", "평균", "1분", "10분", "한시간"};
  kma_data_ex_t *p_kma = NULL;

  switch (mode)
  {
    case 0:
      p_kma = &g_kma_raw_ex;
      break;
    case 1:
      p_kma = &g_kma_inst_ex;
      break;
    case 2:
      p_kma = &g_kma_1min_ex;
      break;
    case 3:
      p_kma = &g_kma_10min_ex;
      break;
    case 4:
      p_kma = &g_kma_1Hour_ex;
      break;
  }

  snprintf(buff, sizeof(buff), "AWS %s %.2fms", aswTitleList[mode],
           (float)g_debug_elased_time / 1000.0f);

#define KMA_TO_TEMPERATURE(x) \
  ((float)((x - 1000) / 10.0f))  //[AWS = (관측값+100)/10, 관측값 = (x-1000)/10]
#define KMA_TO_GENERAL(x) ((float)(x / 10.0f))

#define KMA_TO_1000(x) ((float)((x - 1000) / 10.0f))

#define COL_WIDTH 15
  vt100_print_frame(row, column, buff, '+', '|', '-', DISP_WIDTH, WHITE);

  if (p_kma->temperature.enable)
  {
    if (p_kma->temperature.err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:error\r\n", COL_WIDTH, "기온");
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f C\r\n", COL_WIDTH, "기온",
                      KMA_TO_TEMPERATURE(p_kma->temperature.data));
    }
  }

  if (p_kma->wind_direction_avg.enable)
  {
    if (p_kma->wind_direction_avg.err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:error\r\n", COL_WIDTH, "풍향");
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f 도\r\n", COL_WIDTH, "풍향",
                      KMA_TO_GENERAL(p_kma->wind_direction_avg.data));
    }
  }

  if (p_kma->wind_speed_avg.enable)
  {
    if (p_kma->wind_speed_avg.err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:error\r\n", COL_WIDTH, "풍속");
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
    if (p_kma->pressure.err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:error\r\n", COL_WIDTH, "기압");
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
    if (p_kma->snowfall.err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:error\r\n", COL_WIDTH, "적설");
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5d mm\r\n", COL_WIDTH, "적설",
                      (int)(p_kma->snowfall.data / 10.0f));
    }
  }

  if (p_kma->relative_humidity.enable)
  {
    if (p_kma->relative_humidity.err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:error\r\n", COL_WIDTH, "상대습도");
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.1f %%\r\n", COL_WIDTH, "상대습도",
                      KMA_TO_GENERAL(p_kma->relative_humidity.data));
    }
  }

  if (p_kma->solar_radiation.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f MJ/m2\r\n", COL_WIDTH, "일사",
                    p_kma->solar_radiation.data / 100.f);  // 표현범위	→	0	～
  }

  if (p_kma->sunshine_duration.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5d s\r\n", COL_WIDTH, "일조",
                    (int)p_kma->sunshine_duration.data);  // 표현범위	→	0	～
  }

  if (p_kma->soil_temperature_5cm.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 5cm",
                    KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.data));
  }
  if (p_kma->soil_temperature_10cm.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 10cm",
                    KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.data));
  }
  if (p_kma->soil_temperature_20cm.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 20cm",
                    KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.data));
  }
  if (p_kma->soil_temperature_30cm.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 30cm",
                    KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.data));
  }
  if (p_kma->soil_temperature_50cm.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 50cm",
                    KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.data));
  }
  if (p_kma->soil_temperature_1m.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 1m",
                    KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.data));
  }
  if (p_kma->soil_temperature_1_5m.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 1.5m",
                    KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.data));
  }
  if (p_kma->soil_temperature_3m.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 3m",
                    KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.data));
  }
  if (p_kma->soil_temperature_5m.enable)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 5m",
                    KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.data));
  }
  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

#if 0
  vt100_print_frame(row, column, buff, '+', '|', '-', DISP_WIDTH, WHITE);
  if (p_sensor[A1_TEMPERATURE].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "기온          :%5.1f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->temperature));
  if (p_sensor[A2_WIND_DIRECTION].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "풍향          :%5.1f 도\r\n",
                    KMA_TO_GENERAL(pkma->wind_direction_avg));
  if (p_sensor[A3_WIND_SPEED].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "풍속          :%5.1f m/s\r\n",
                    KMA_TO_GENERAL(pkma->wind_speed_avg));
  if (p_sensor[A4_INSTANT_WIND_DIRECTION].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "순간 풍향     :%5.1f 도\r\n",
                    KMA_TO_GENERAL(pkma->wind_direction_instant));
  if (p_sensor[A5_INSTANT_WIND_SPEED].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "순간 풍속     :%5.1f m/s\r\n",
                    KMA_TO_GENERAL(pkma->wind_speed_instant));
  if (p_sensor[A6_RAINFALL_DOT5_1MM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "강수량        :%5.1f mm\r\n",
                    (float)(pkma->precipitation / 10.0f));
  if (p_sensor[A7_PRESSURE].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "기압          :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->pressure));
  if (p_sensor[A8_RAIN_PRESENT].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "강수유무      :%5s\r\n",
                    pkma->precipitation_presence == 10 ? "유" : "무");
  if (p_sensor[A9_SNOW_DEPTH].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "적설          :%5d mm\r\n",
                    (int)(pkma->snowfall / 10.0f));
  if (p_sensor[A10_RELATIVE_HUMIDITY].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "상대습도      :%5.1f %%\r\n",
                    KMA_TO_GENERAL(pkma->relative_humidity));
  if (p_sensor[A11_RAINFALL_DOT1MM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "강수량        :%5d mm\r\n",
                    pkma->precipitation_fine);
  if (p_sensor[B1_SOLAR_RADIATION].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "일사          :%5.2f MJ/m2\r\n",
                    pkma->solar_radiation / 100.f);  // 표현범위	→	0	～
                                                        // 32767 [누적	값(MJ/m2)	×	100]
  if (p_sensor[B2_SUNSHINE_DURATION].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "일조          :%5d s\r\n",
                    (int)pkma->sunshine_duration);  // 표현범위	→	0	～
                                                       // 65535	[누적시간(초	단위)
  if (p_sensor[B3_GROUND_TEMPERATURE].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지면온도      :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->surface_temperature));  // 표현범위	→	500
                                                                        // ～	2000	[(관측값
                                                                        // ＋	100)	×	10
  if (p_sensor[B4_SURFACE_TEMPERATURE].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "초상온도      :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->grass_temperature));
  if (p_sensor[B5_SOIL_TEMPERATURE_5CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지중온도 5cm  :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_5cm));
  if (p_sensor[B6_SOIL_TEMPERATURE_10CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지중온도 10cm :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_10cm));
  if (p_sensor[B7_SOIL_TEMPERATURE_20CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지중온도 20cm :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_20cm));
  if (p_sensor[B8_SOIL_TEMPERATURE_30CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지중온도 30cm :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_30cm));
  if (p_sensor[B9_SOIL_TEMPERATURE_50CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지중온도 50cm :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_50cm));
  if (p_sensor[B10_SOIL_TEMPERATURE_100CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지중온도 1m   :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_1m));
  if (p_sensor[B11_SOIL_TEMPERATURE_150CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지중온도 1.5m :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_1_5m));
  if (p_sensor[B12_SOIL_TEMPERATURE_300CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지중온도 3m   :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_3m));
  if (p_sensor[B13_SOIL_TEMPERATURE_500CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "지중온도 5cm  :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_5m));
  if (p_sensor[C1_CLOUD_BASE1].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "1층 운고      :%dm\r\n",
                    (pkma->cloud_height_1st));  // 표현범위	→	1
                                                   // ～ 8000	[관측값(m)]
  if (p_sensor[C2_CLOUD_BASE2].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "2층 운고      :%dm\r\n",
                    (pkma->cloud_height_2nd));  // 표현범위	→	1
                                                   // ～ 8000	[관측값(m)]
  if (p_sensor[C3_CLOUD_BASE3].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "3층 운고      :%dm\r\n",
                    (pkma->cloud_height_3rd));  // 표현범위	→	1
                                                   // ～ 8000	[관측값(m)]
  if (p_sensor[27].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "운량          :%5d\r\n",
                    (pkma->cloud_amount));  // 표현범위	→	0
                                               // ～	10	(관측값)
  if (p_sensor[28].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "시정          :%5d m\r\n",
                    (pkma->visibility));  // 표현범위	→	1
                                             // ～	50000	[관측값(m)]
  if (p_sensor[29].type)
    vt100_print_bar(
        line++, column, -DISP_WIDTH, "PM10          :%5.1f μg/㎥\r\n",
        KMA_TO_GENERAL(pkma->pm10_concentration));  // 표현범위	→	1	～
                                                       // 3599	[관측값(μg/㎥)	×	10]
  if (p_sensor[30].type)
    vt100_print_bar(
        line++, column, -DISP_WIDTH, "PM2.5         :%5.1f μg/㎥\r\n",
        KMA_TO_GENERAL(pkma->pm25_concentration));  // 표현범위	→	1	～
                                                       // 3599	[관측값(μg/㎥)	×	10]
  if (p_sensor[31].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "순복사        :%5f W/m2\r\n",
                    KMA_TO_1000(pkma->net_radiation));  // 표현범위	→	0
                                                           // ～	32767	{[관측값(W/m2)
                                                           // +	1000]	×	10}
  if (p_sensor[32].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "전천복사      :%5f W/m2\r\n",
                    KMA_TO_1000(pkma->total_radiation));  // 표현범위	→	0
                                                             // ～	32767	{[관측값(W/m2)
                                                             // +	1000]	×	10}
  if (p_sensor[33].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "반사복사      :%5f W/m2\r\n",
                    KMA_TO_1000(pkma->reflected_radiation));  // 표현범위	→	0
                                                                 // ～	32767	{[관측값(W/m2)
                                                                 // +	1000]	×	10}
  if (p_sensor[34].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "직달일사      :%5f W/m2\r\n",
                    KMA_TO_1000(pkma->direct_radiation));  // 표현범위	→	0
                                                              // ～	32767	{[관측값(W/m2)
                                                              // +	1000]	×	10}
  if (p_sensor[35].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "현재일기      :%5f\r\n",
                    pkma->current_weather);  // 표현범위	→	0
                                                // ～	99	(관측값)
  if (p_sensor[36].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "토양수분10cm  :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->soil_moisture_10cm));  // 표현범위	→	0	～
                                                                   // 1000	(관측값	×	10)
  if (p_sensor[37].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "토양수분20cm  :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->soil_moisture_20cm));  // 표현범위	→	0	～
                                                                   // 1000	(관측값	×	10)
  if (p_sensor[38].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "토양수분30cm  :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->soil_moisture_30cm));  // 표현범위	→	0	～
                                                                   // 1000	(관측값	×	10)
  if (p_sensor[39].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "토양수분50cm  :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->soil_moisture_50cm));  // 표현범위	→	0	～
                                                                   // 1000	(관측값	×	10)
  if (p_sensor[40].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "조도량        :%5.2f klux\r\n",
                    (pkma->illuminance));  // 표현범위	→	0	～	32767
                                              // (관측값(klux)	×	100)
  if (p_sensor[N6_WIND_VELOCITY_150CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "풍속1.5m      :%5.2f m/s\r\n",
                    KMA_TO_GENERAL(pkma->wind_speed_1_5m));  // 표현범위	→	1	～
                                                                // 1000	(관측값	×	10)
  if (p_sensor[42].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "풍속4m        :%5.2f m/s\r\n",
                    KMA_TO_GENERAL(pkma->wind_speed_4m));  // 표현범위	→	1	～
                                                              // 1000	(관측값	×	10)
  if (p_sensor[43].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "순간풍속1.5m  :%5.2f m/s\r\n",
                    KMA_TO_GENERAL(pkma->instant_wind_speed_1_5m));  // 표현범위	→
                                                                        // 1 ～	1000
                                                                        // (관측값	× 10)
  if (p_sensor[44].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "순간풍속4m    :%5.2f m/s\r\n",
                    KMA_TO_GENERAL(pkma->instant_wind_speed_4m));  // 표현범위	→
                                                                      // 1	～ 1000	(관측값
                                                                      // ×	10)
  if (p_sensor[45].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "기온0.5m      :%5.2f C\r\n",
                    (pkma->temperature_0_5m));  // 표현범위	→	500	～	1500
                                                   // [(관측값	＋	100)	×	10
  if (p_sensor[46].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "기온4m        :%5.2f C\r\n",
                    (pkma->temperature_4m));  // 표현범위	→	500	～	1500
                                                 // [(관측값	＋	100)	×	10
  if (p_sensor[47].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "습도0.5m      :%5.2f %%\r\n",
                    (pkma->humidity_0_5m));  // 표현범위	→	500	～	1500
                                                // [(관측값	＋	100)	×	10
  if (p_sensor[48].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "습도4m        :%5.2f %%\r\n",
                    (pkma->humidity_4m));  // 표현범위	→	500	～	1500
                                              // [(관측값	＋	100)	×	10
  if (p_sensor[49].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "타코4m        :%5d\r\n",
                    (pkma->tacometer));  // 표현범위	→	0	～	8000	(관측값)

  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);
#endif
  return 5 + 2;
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
    print_awsRealLefinfo(1, 60, awsMode, NULL);

    key = get_key(100);

    if (key == KEY_CODE_RIGHT)
    {
      if (awsMode < AWS_MODE_MAX)
      {
        awsMode++;
      }
    }
    else if (key == KEY_CODE_LEFT)
    {
      if (awsMode > 0)
      {
        awsMode--;
      }
    }

    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
  } while (1);

  vt100_print(50, 0, "\r\n");
  debug_printf(VT100_CURSOR_ON);
  return 0;
}