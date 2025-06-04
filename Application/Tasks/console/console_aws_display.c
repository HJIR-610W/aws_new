

#include "console_aws_display.h"
#include "vt100_command.h"
#include "config_app.h"
#include "dev_io.h"
#include "bsp.h"
#include "task_tcpServer.h"
#include "task_direct.h"
#include "util_time.h"
#include "aws_data.h"
#include "console_utile.h"
#include "app_charger.h"
#include "bsp_di.h"
#include "app_bsp.h"
#include "task_logging.h"
#include "cli_key_code.h"
#include "task_measure.h"
#include "dualport.h"
#include "task_logging.h"
#include "task_system.h"
#include "task_cellular.h"
#include "task_client.h"


#define AWS_MODE_MAX 4

#define DISP_WIDTH 30

const char *linkStatusList[] = {"-", "up", "down"};
const char *doorStatusList[] = {"닫힘", "열림"};
const char *generalStatusList[] = {"정상", "비정상"};


extern uint32_t g_debug_elased_time;

void make_error_string(uint8_t error, char *buffer, uint32_t buffer_size)
{
  if (!buffer || buffer_size == 0)
    return;

  uint8_t val_err = (error >> 4) & 0x0F;
  uint8_t comm_err = error & 0x0F;

  if (val_err == 0 && comm_err == 0)
  {
    buffer[0] = '\0';  // 에러 없음
    return;
  }

  if (val_err && comm_err)
  {
    snprintf(buffer, buffer_size, "E(값%u,통신%u)", val_err, comm_err);
  }
  else if (val_err)
  {
    snprintf(buffer, buffer_size, "E(값%u)", val_err);
  }
  else if (comm_err)
  {
    snprintf(buffer, buffer_size, "E(통신%u)", comm_err);
  }
}



int32_t print_systemInfo(uint16_t row, uint16_t column)
{
  uint8_t line = row + 3;
  char buff[30];
  const char *message=NULL;

  snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d\r\n", Date_Time.Year, Date_Time.Month,
           Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);

  vt100_print_frame(row, column, "시스템", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "%s\r\n", buff);
  vt100_print_bar(line++, column, -DISP_WIDTH, "ID        :%d\r\n",get_config_app()->id);
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

  vt100_print_bar(line++, column, -DISP_WIDTH, "장비 전원 :%5.2f V\r\n", bsp_read_battery());
  vt100_print_bar(line++, column, -DISP_WIDTH, "장비 온도 :%5.2f C\r\n", bsp_read_temperature());

  if(get_config_app()->ac_use)
  {
  vt100_print_bar(line++, column, -DISP_WIDTH, "AC        :정상\r\n");
  }
  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line;
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

  return line;
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
  eLINK_STATUS_t link_status;
  uint8_t tx_cnt;
  uint8_t rx_cnt;
  uint8_t line = row + 3;

  if (get_config_app()->eth_mode == eETH_MODE_CLINET)
  {
    link_status = get_tcp_client_system()->link_status;
    tx_cnt = get_tcp_client_system()->tx_cnt;
    rx_cnt = get_tcp_client_system()->rx_cnt;
  }
  else
  {
    link_status = get_tcp_system()->link_status;
    tx_cnt = get_tcp_system()->tx_cnt;
    rx_cnt = get_tcp_system()->rx_cnt;
  }
        make_comList(buff, sizeof(buff));
  vt100_print_frame(row, column, "이더넷", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "링크  :%s\r\n",
                  ITEM_LIST(link_status , linkStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "송신  :%d\r\n", tx_cnt);
  vt100_print_bar(line++, column, -DISP_WIDTH, "수신  :%d\r\n", rx_cnt);

  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line ;
}


/*
10분
온도
기압
습도
풍향
풍속
일사
일조
지중온도 5cm
지중온도 10cm
지중온도 20cm
지중온도 30cm
지중온도 50cm
지중온도 1_0m
지둥온도 1_5m
10분 강수량
*/

/*
+--------------------------+
|           CDMA           |
+--------------------------+
|링크    :down             |
|전화번호:-                |
|수신감도:0                |
|송신    :0                |
|수신    :0                |
|T시간:2025-25-11 00:00:00 |
|R시간:2025-25-11 00:00:00 |
+--------------------------+
*/
int32_t print_cdmaInfo(uint16_t row, uint16_t column)
{
  char buff[30];
  char num[20];
  uint8_t line = row + 3;

  DATE_TIME_BUF nt;
  uint32_t last_time;

  make_comList(buff, sizeof(buff));
  vt100_print_frame(row, column, "CDMA", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "링크    :%s\r\n",
                    ITEM_LIST(get_cdma_system()->link_status, linkStatusList));

  if (get_cdma_system()->num[0] != '0')
  {
    num[0] = '-';
    num[1] = 0;
  }
  else
  {
    snprintf(num, sizeof(num), "%s", get_cdma_system()->num);
  }
  vt100_print_bar(line++, column, -DISP_WIDTH, "전화번호:%s\r\n", num);
  if (get_cdma_system()->rssi == -1)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "수신감도:-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "수신감도:%d\r\n", get_cdma_system()->rssi);
  }

  vt100_print_bar(line++, column, -DISP_WIDTH, "송신    :%d\r\n", get_cdma_system()->tx_cnt);
  vt100_print_bar(line++, column, -DISP_WIDTH, "수신    :%d\r\n", get_cdma_system()->rx_cnt);

  last_time = get_cdma_system()->last_recv_time;
  time_cvt_secTotime(last_time, &nt);
  if(last_time==0)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "R시간   :-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "R시간   :%02d-%02d-%2d %02d:%02d:%02d\r\n",
                    nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  last_time = get_cdma_system()->last_send_time;
  time_cvt_secTotime(last_time, &nt);
  if(last_time==0)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "T시간   :-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "T시간   :%02d-%02d-%2d %02d:%02d:%02d\r\n",
                    nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line ;
}


int32_t print_directInfo(uint16_t row, uint16_t column)
{
  char buffer[30];
  char num[20];
  uint8_t line = row + 3;
  int8_t rssi;
  DATE_TIME_BUF nt;
  uint32_t last_time;
  struct tm time_info;
  uint32_t remain_sec;

  remain_sec = (uint32_t)(get_direct_system()->linkdown_remain_ms/1000.0);
   make_comList(buffer, sizeof(buffer));
  vt100_print_frame(row, column, "DIRECT", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "링크    :%s\r\n",
                  ITEM_LIST(get_direct_system()->link_status, linkStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "타임아웃:%ds\r\n",remain_sec);
  vt100_print_bar(line++, column, -DISP_WIDTH, "송신    :%d\r\n", get_direct_system()->tx_cnt);
  vt100_print_bar(line++, column, -DISP_WIDTH, "수신    :%d\r\n", get_direct_system()->rx_cnt);

  last_time = get_direct_system()->last_recv_time;
  
  if(last_time == 0)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "R시간   :-\r\n");
  }
  else
  {
    time_cvt_secTotime(last_time, &nt);

    vt100_print_bar(line++, column, -DISP_WIDTH, "R시간   :%02d-%02d-%2d %02d:%02d:%02d\r\n",
                    nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  last_time = get_direct_system()->last_send_time;

  if(last_time == 0 )
  {
    time_cvt_secTotime(last_time, &nt);
    vt100_print_bar(line++, column, -DISP_WIDTH, "T시간   :-\r\n");
  }
  else
  {
    time_cvt_secTotime(last_time, &nt);
    vt100_print_bar(line++, column, -DISP_WIDTH, "T시간   :%02d-%02d-%2d %02d:%02d:%02d\r\n",
                    nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }


  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line;
}

//[AWS = (관측값+100)/10, 관측값 = (x-1000)/10]
#define KMA_TO_TEMPERATURE(x) ((float)((x - 1000) / 10.0f))  
#define KMA_TO_GENERAL(x) ((float)(x / 10.0f))
#define KMA_TO_1000(x) ((float)((x - 1000) / 10.0f))
#define KMA_TO_ILLUMINANCE(x) ((x) / 100.0f)
#define KMA_TO_RADI(x) ((x) / 10.0f - 100.0f)

#define COL_WIDTH 15


int32_t print_awsRealLefinfo(uint16_t row, uint16_t column, eAWS_DATA_MIN_t min, void *arg)
{
  const char *aswTitleList[] = {"순간(평균)", "1분", "10분", "한시간","RAW"};
  char buff[50];
  uint8_t err;
  uint8_t line = row + 3;
  kma_data_ex_t *p_kma = NULL;
  uint32_t elapsed_time;
  char err_buf[32];

  p_kma = get_kma_data(min);


  elapsed_time = g_exec_250ms_time.elapsed_time + g_exec_1s_time.elapsed_time;

  snprintf(buff, sizeof(buff), "AWS %s %.2fms", aswTitleList[min], (float)elapsed_time / 1000.0f);

  vt100_print_frame(row, column, buff, '+', '|', '-', DISP_WIDTH, WHITE);
// 자동 생성된 AWS 출력 코드
if(p_kma->temperature.enable)
{
  err = p_kma->temperature.err;
  if (err)
  {
    make_error_string(err,err_buf,sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "기온", err_buf);
  }
  else
  {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "기온",
                      KMA_TO_TEMPERATURE(p_kma->temperature.data));
  }
}

if (p_kma->wind_direction_avg.enable)
{
  err = p_kma->wind_direction_avg.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "풍향", err_buf);
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f 도\r\n", COL_WIDTH, "풍향",
                    KMA_TO_GENERAL(p_kma->wind_direction_avg.data));
  }
}

if (p_kma->wind_speed_avg.enable)
{
  err = p_kma->wind_speed_avg.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "풍속", err_buf);
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m/s\r\n", COL_WIDTH, "풍속",
                    KMA_TO_GENERAL(p_kma->wind_speed_avg.data));
  }
}

if (p_kma->wind_direction_avg.enable)
{
  err = p_kma->wind_direction_avg.err;
  if (err)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "순간 풍향");
  }
  else
  {
    if (min == eAWS_DATA_RAW)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "순간 풍향");
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f 도\r\n", COL_WIDTH, "순간 풍향",
                      KMA_TO_GENERAL(p_kma->wind_direction_instant.data));
    }
  }
  }

  if (p_kma->wind_speed_avg.enable)
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "순간 풍속");
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "순간 풍속");
      }
      else
      {
        vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m/s\r\n", COL_WIDTH, "순간 풍속",
                        KMA_TO_GENERAL(p_kma->wind_speed_instant.data));
      }
    }
}


if (p_kma->precipitation.enable)
{
  err = p_kma->precipitation.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "강수량",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_RAW)
    {
      uint32_t last_time;
      last_time = p_kma->precipitation.last_time;
      DATE_TIME_BUF nt;
      char buff[20];
      if(last_time==0)//우량이 내린적이 없으면
      {
        vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "강수량(time)");
      }
      else
      {
        time_cvt_secTotime(last_time,&nt);
        vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%04d%02d%02d%02d%02d%02d\r\n", COL_WIDTH,
                        "강수량(time)", nt.Year,nt.Month,nt.Day,nt.Hour,nt.Min,nt.Sec);
      }
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f mm\r\n", COL_WIDTH, "강수량",
                      KMA_TO_GENERAL(p_kma->precipitation.data));
    }
  }
}

if (p_kma->pressure.enable)
{
  err = p_kma->pressure.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "기압", err_buf);
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f hPa\r\n", COL_WIDTH, "기압",
                    KMA_TO_GENERAL(p_kma->pressure.data));
  }
}

if (p_kma->precipitation_presence.enable)
{
  err = p_kma->precipitation_presence.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "강수유무",
                    err_buf);
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5d \r\n", COL_WIDTH, "강수유무",
                    p_kma->precipitation_presence.data);
  }
}

if (p_kma->snowfall.enable)
{
  err = p_kma->snowfall.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "적설", err_buf);
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f mm\r\n", COL_WIDTH, "적설",
                    KMA_TO_GENERAL(p_kma->snowfall.data));
  }
}

if (p_kma->relative_humidity.enable)
{
  err = p_kma->relative_humidity.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "상대습도", err_buf);
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f %\r\n", COL_WIDTH, "상대습도",
                    KMA_TO_GENERAL(p_kma->relative_humidity.data));
  }
}

if (p_kma->precipitation_fine.enable)
{
  err = p_kma->precipitation_fine.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "강수량(0.1)", err_buf);
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f mm\r\n", COL_WIDTH, "강수량(0.1)",
                    KMA_TO_GENERAL(p_kma->precipitation_fine.data));
  }
}

if (p_kma->solar_radiation.enable)
{
  err = p_kma->solar_radiation.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "일사", err_buf);
  }
  else
  {
    float solar_radiation;

    solar_radiation = p_kma->solar_radiation.data;

    switch (min)
    {
      case eAWS_DATA_RAW:
      case eAWS_DATA_AVG:
       vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f WJ/m2\r\n",
                                         COL_WIDTH, "일사", solar_radiation);
          break;

          default:
        vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f KJ/m2\r\n", COL_WIDTH, "일사",
                        solar_radiation);
        break;

    }

  }
}

if (p_kma->sunshine_duration.enable)
{
  err = p_kma->sunshine_duration.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "일조", err_buf);
  }
  else
  {
    switch (min)
    {
      case eAWS_DATA_RAW:
      case eAWS_DATA_AVG:
      {
        bool sunshine_duration;
        sunshine_duration = (p_kma->sunshine_duration.data == 1);
        vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "일조",
                        sunshine_duration ? "ON" : "OFF");
      }
        break;

      default:
        vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5d s\r\n", COL_WIDTH, "일조",
                        p_kma->sunshine_duration.data);
        break;
    }

  }
}

if (p_kma->surface_temperature.enable)
{
  err = p_kma->surface_temperature.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지면온도", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지면온도",
                      KMA_TO_TEMPERATURE(p_kma->surface_temperature.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f C");
    }
  }
}

if (p_kma->grass_temperature.enable)
{
  err = p_kma->grass_temperature.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "초상온도", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "초상온도",
                      KMA_TO_TEMPERATURE(p_kma->grass_temperature.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f C");
    }
  }
}

if (p_kma->soil_temperature_5cm.enable)
{
  err = p_kma->soil_temperature_5cm.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지중온도 5cm", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 5cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "지중온도 5cm");
    }
  }
}

if (p_kma->soil_temperature_10cm.enable)
{
  err = p_kma->soil_temperature_10cm.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지중온도 10cm", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 10cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "지중온도 10cm");
    }
  }
}

if (p_kma->soil_temperature_20cm.enable)
{
  err = p_kma->soil_temperature_20cm.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지중온도 20cm", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 20cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "지중온도 20cm");
    }
  }
}

if (p_kma->soil_temperature_30cm.enable)
{
  err = p_kma->soil_temperature_30cm.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지중온도 30cm", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 30cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "지중온도 30cm");
    }
  }
}

if (p_kma->soil_temperature_50cm.enable)
{
  err = p_kma->soil_temperature_50cm.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지중온도 50cm", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 50cm",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "지중온도 50cm");
    }
  }
}

if (p_kma->soil_temperature_1m.enable)
{
  err = p_kma->soil_temperature_1m.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지중온도 1m", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 1m",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "지중온도 1m");
    }
  }
}

if (p_kma->soil_temperature_1_5m.enable)
{
  err = p_kma->soil_temperature_1_5m.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지중온도 1.5m", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 1.5m",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "지중온도 1.5m");
    }
  }
}

if (p_kma->soil_temperature_3m.enable)
{
  err = p_kma->soil_temperature_3m.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지중온도 3m", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 3m",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "지중온도 3m");
    }
  }
}

if (p_kma->soil_temperature_5m.enable)
{
  err = p_kma->soil_temperature_5m.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "지중온도 5m", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "지중온도 5m",
                      KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "지중온도 5m");
    }
  }
}

if (p_kma->cloud_height_1st.enable)
{
  err = p_kma->cloud_height_1st.err;
  if (err)
  {
    
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "운고 1층", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m\r\n", COL_WIDTH, "운고 1층",
                      KMA_TO_GENERAL(p_kma->cloud_height_1st.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f m");
    }
  }
}

if (p_kma->cloud_height_2nd.enable)
{
  err = p_kma->cloud_height_2nd.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "운고 2층", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m\r\n", COL_WIDTH, "운고 2층",
                      KMA_TO_GENERAL(p_kma->cloud_height_2nd.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f m");
    }
  }
}

if (p_kma->cloud_height_3rd.enable)
{
  err = p_kma->cloud_height_3rd.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "운고 3층", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m\r\n", COL_WIDTH, "운고 3층",
                      KMA_TO_GENERAL(p_kma->cloud_height_3rd.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f m");
    }
  }
}

if (p_kma->cloud_amount.enable)
{
  err = p_kma->cloud_amount.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "운량", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f \r\n", COL_WIDTH, "운량",
                      KMA_TO_GENERAL(p_kma->cloud_amount.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f");
    }
  }
}

if (p_kma->visibility.enable)
{
  err = p_kma->visibility.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "시정", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m\r\n", COL_WIDTH, "시정",
                      KMA_TO_GENERAL(p_kma->visibility.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f m");
    }
  }
}

if (p_kma->pm10_concentration.enable)
{
  err = p_kma->pm10_concentration.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "미세먼지 PM10",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f ug/m3\r\n", COL_WIDTH,
                      "미세먼지 PM10", KMA_TO_GENERAL(p_kma->pm10_concentration.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f ug/m3");
    }
  }
}

if (p_kma->pm25_concentration.enable)
{
  err = p_kma->pm25_concentration.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "미세먼지 PM2.5",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f ug/m3\r\n", COL_WIDTH,
                      "미세먼지 PM2.5", KMA_TO_GENERAL(p_kma->pm25_concentration.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f ug/m3");
    }
  }
}

if (p_kma->net_radiation.enable)
{
  err = p_kma->net_radiation.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "순복사", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f W/m2\r\n", COL_WIDTH, "순복사",
                      KMA_TO_RADI(p_kma->net_radiation.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f W/m2");
    }
  }
}

if (p_kma->total_radiation.enable)
{
  err = p_kma->total_radiation.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "전천복사", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f W/m2\r\n", COL_WIDTH, "전천복사",
                      KMA_TO_RADI(p_kma->total_radiation.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f W/m2");
    }
  }
}

if (p_kma->reflected_radiation.enable)
{
  err = p_kma->reflected_radiation.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "반사복사", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f W/m2\r\n", COL_WIDTH, "반사복사",
                      KMA_TO_RADI(p_kma->reflected_radiation.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f W/m2");
    }
  }
}

if (p_kma->direct_radiation.enable)
{
  err = p_kma->direct_radiation.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "직달일사", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f W/m2\r\n", COL_WIDTH, "직달일사",
                      KMA_TO_RADI(p_kma->direct_radiation.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f W/m2");
    }
  }
}

if (p_kma->current_weather.enable)
{
  err = p_kma->current_weather.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "현재일기", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f \r\n", COL_WIDTH, "현재일기",
                      KMA_TO_GENERAL(p_kma->current_weather.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f");
    }
  }
}

if (p_kma->soil_moisture_10cm.enable)
{
  err = p_kma->soil_moisture_10cm.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "토양수분 10cm",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f %\r\n", COL_WIDTH, "토양수분 10cm",
                      KMA_TO_GENERAL(p_kma->soil_moisture_10cm.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f %");
    }
  }
}

if (p_kma->soil_moisture_20cm.enable)
{
  err = p_kma->soil_moisture_20cm.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "토양수분 20cm",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f %\r\n", COL_WIDTH, "토양수분 20cm",
                      KMA_TO_GENERAL(p_kma->soil_moisture_20cm.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f %");
    }
  }
}

if (p_kma->soil_moisture_30cm.enable)
{
  err = p_kma->soil_moisture_30cm.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "토양수분 30cm",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f %\r\n", COL_WIDTH, "토양수분 30cm",
                      KMA_TO_GENERAL(p_kma->soil_moisture_30cm.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f %");
    }
  }
}

if (p_kma->soil_moisture_50cm.enable)
{
  err = p_kma->soil_moisture_50cm.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "토양수분 50cm",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f %\r\n", COL_WIDTH, "토양수분 50cm",
                      KMA_TO_GENERAL(p_kma->soil_moisture_50cm.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f %");
    }
  }
}

if (p_kma->illuminance.enable)
{
  err = p_kma->illuminance.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "조도", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f klux\r\n", COL_WIDTH, "조도",
                      KMA_TO_ILLUMINANCE(p_kma->illuminance.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f klux");
    }
  }
}

if (p_kma->wind_speed_1_5m.enable)
{
  err = p_kma->wind_speed_1_5m.err;
  if (err)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "풍속 1.5m",
      err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m/s\r\n", COL_WIDTH, "풍속 1.5m",
                      KMA_TO_GENERAL(p_kma->wind_speed_1_5m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f m/s");
    }
  }
}

if (p_kma->wind_speed_4m.enable)
{
  err = p_kma->wind_speed_4m.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "풍속 4.0m", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m/s\r\n", COL_WIDTH, "풍속 4.0m",
                      KMA_TO_GENERAL(p_kma->wind_speed_4m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f m/s");
    }
  }
}

if (p_kma->instant_wind_speed_1_5m.enable)
{
  err = p_kma->instant_wind_speed_1_5m.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "순간풍속 1.5m",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m/s\r\n", COL_WIDTH, "순간풍속 1.5m",
                      KMA_TO_GENERAL(p_kma->instant_wind_speed_1_5m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f m/s");
    }
  }
}

if (p_kma->instant_wind_speed_4m.enable)
{
  err = p_kma->instant_wind_speed_4m.err;
  if (err)
  {
    make_error_string(err, err_buf, sizeof(err_buf));
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "순간풍속 4.0m",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f m/s\r\n", COL_WIDTH, "순간풍속 4.0m",
                      KMA_TO_GENERAL(p_kma->instant_wind_speed_4m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f m/s");
    }
  }
}

if (p_kma->temperature_0_5m.enable)
{
  err = p_kma->temperature_0_5m.err;
  if (err)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "기온 0.5m",
      err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "기온 0.5m",
                      KMA_TO_TEMPERATURE(p_kma->temperature_0_5m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f C");
    }
  }
}

if (p_kma->temperature_4m.enable)
{
  err = p_kma->temperature_4m.err;
  if (err)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "기온 4.0m",
      err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f C\r\n", COL_WIDTH, "기온 4.0m",
                      KMA_TO_TEMPERATURE(p_kma->temperature_4m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "%5.2f C");
    }
  }
}

if (p_kma->humidity_0_5m.enable)
{
  err = p_kma->humidity_0_5m.err;
  if (err)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "습도 0.5m",
      err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f %\r\n", COL_WIDTH, "습도 0.5m",
                      KMA_TO_GENERAL(p_kma->humidity_0_5m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "습도 0.5m");
    }
  }
}

if (p_kma->humidity_4m.enable)
{
  err = p_kma->humidity_4m.err;
  if (err)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "습도 4.0m",
                    err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f %\r\n", COL_WIDTH, "습도 4.0m",
                      KMA_TO_GENERAL(p_kma->humidity_4m.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "습도 4.0m");
    }
  }
}

if (p_kma->tacometer.enable)
{
  err = p_kma->tacometer.err;
  if (err)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%s\r\n", COL_WIDTH, "타코미터", err_buf);
  }
  else
  {
    if (min == eAWS_DATA_AVG)
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:%5.2f rpm\r\n", COL_WIDTH, "타코미터",
                      KMA_TO_GENERAL(p_kma->tacometer.data));
    }
    else
    {
      vt100_print_bar(line++, column, -DISP_WIDTH, "%-*s:--\r\n", COL_WIDTH, "타코미터");
    }
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

  io_printf(VT100_CLEAR_SCREEN);
  io_printf(VT100_CURSOR_OFF);

  do
  {
    io_printf(VT100_CURSOR_HOME);
    io_printf("\r\n");
    
    line = 0;
    line = print_systemInfo(1, 0);

    print_chargerInfo(line+1, 0);

    line = 0;
    if (get_config_app()->cdma_use)
    {
      line = print_cdmaInfo(1, DISP_WIDTH+3);
    }
    if (get_config_app()->direct_use)
    {
      line += print_directInfo(1 + line, DISP_WIDTH+3);
    }
    if (get_config_app()->eth_use)
    {
      line += print_ethInfo(1 + line, DISP_WIDTH+3);
    }

    line = 0;
    line = print_awsRealLefinfo(1, (DISP_WIDTH+3)*2, (eAWS_DATA_MIN_t)awsMode, NULL);

    print_rainInfo(1 + line, (DISP_WIDTH + 3) * 2);

    key = (keycode_t)get_key(500);

    if (key == KEY_CODE_RIGHT)
    {
      io_printf(VT100_CLEAR_SCREEN);
      if (awsMode < AWS_MODE_MAX)
      {
        awsMode++;
      }
    }
    else if (key == KEY_CODE_LEFT)
    {
      io_printf(VT100_CLEAR_SCREEN);
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
  io_printf(VT100_CURSOR_ON);
  return 0;
}