

#include "aws_menu_display.h"

#include "app_charger.h"
#include "aws_data.h"
#include "bsp.h"
#include "bsp_di.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "console_utile.h"
#include "dev_io.h"
#include "dualport.h"
#include "task_cellular.h"
#include "task_client.h"
#include "task_direct.h"
#include "task_logging.h"
#include "task_measure.h"
#include "task_system.h"
#include "task_tcpServer.h"
#include "util_time.h"
#include "vt100_command.h"

#define AWS_MODE_MAX 4

#define DISP_WIDTH 30

#define SMALL_W 27
const char *linkStatusList[] = {"-", "UP", "DOWN"};
const char *doorStatusList[] = {"닫힘", "열림"};
const char *generalStatusList[] = {"정상", "비정상"};

static uint8_t s_navi=0;
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


int32_t print_system_info(uint16_t row, uint16_t column, uint8_t selected)
{
  
  uint8_t line = row + 4;
  char buff[30];
  const char *message=NULL;

  snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d\r\n", Date_Time.Year, Date_Time.Month,
           Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);

  vt100_print_frame_selected(row+1, column, "시스템", '+', '|', '-', SMALL_W, WHITE, selected);
  vt100_print_bar(line++, column, -SMALL_W, "%s\r\n", buff);
  vt100_print_bar(line++, column, -SMALL_W, "ID        :%d\r\n", get_config_app()->id);
  vt100_print_bar(line++, column, -SMALL_W, "문 상태   :%s\r\n",
                  ITEM_LIST(IS_DOOR_OPENED(), doorStatusList));

  if(get_logging_system()->status_group)
  {
    message = "오류";
  }
  else
  {
    message = "정상";
  }

  vt100_print_bar(line++, column, -SMALL_W, "저장 기능 :%s\r\n", message);

  vt100_print_bar(line++, column, -SMALL_W, "장비 전원 :%5.2f V\r\n", bsp_read_battery());
  vt100_print_bar(line++, column, -SMALL_W, "장비 온도 :%5.2f C\r\n", bsp_read_temperature());

  if(get_config_app()->ac_use)
  {
    vt100_print_bar(line++, column, -SMALL_W, "AC        :정상\r\n");
  }

  vt100_print_bar(line++, column, -SMALL_W, " \r\n");
  vt100_print_line(line++, column, '+', '-', SMALL_W);

  return line - (row+1);
}


int32_t print_charger_info(uint16_t row, uint16_t column, uint8_t selected)
{
  char buff[10];

  uint8_t line = row + 4;
  uint8_t err;

  read_chargerStatus(buff, sizeof(buff));
  vt100_print_frame_selected(row+1, column, "충전기", '+', '|', '-', SMALL_W, WHITE, selected);
  vt100_print_bar(line++, column, -SMALL_W, "상태           :%s\r\n", buff);

  if (is_chargerValid())
  {
    vt100_print_bar(line++, column, -SMALL_W, "  충전 전압(V) :%.2f\r\n", read_solarVoltage1(&err));
    vt100_print_bar(line++, column, -SMALL_W, "  충전 전류(A) :%.2f\r\n", read_solarCurrrent1(&err));
    vt100_print_bar(line++, column, -SMALL_W, "배터리 전압(V) :%.2f\r\n", read_batteryVoltage1(&err));
    vt100_print_bar(line++, column, -SMALL_W, " 부하1 전류(A) :%.2f\r\n", read_loadCurrent1(&err));
  }
  else
  {
    vt100_print_bar(line++, column, -SMALL_W, "  충전 전압(V) :--\r\n");
    vt100_print_bar(line++, column, -SMALL_W, "  충전 전류(A) :--\r\n");
    vt100_print_bar(line++, column, -SMALL_W, "배터리 전압(V) :--\r\n");
    vt100_print_bar(line++, column, -SMALL_W, " 부하1 전류(A) :--\r\n");
  }
  vt100_print_bar(line++, column, -SMALL_W, " \r\n");
  vt100_print_bar(line++, column, -SMALL_W, " \r\n");
  vt100_print_line(line++, column, '+', '-', SMALL_W);

  return line - (row+1);
}


int32_t print_rain_info(uint16_t row, uint16_t column,uint8_t selected)
{
  char buff[30];

  uint8_t line = row + 4;

  make_comList(buff, sizeof(buff));
  vt100_print_frame_selected(row+1, column, "강수량", '+', '|', '-', SMALL_W, WHITE, selected);
  vt100_print_bar(line++, column, -SMALL_W, "전일:%6.1f\r\n", get_rainfall()->rainfall_yesterday);
  vt100_print_bar(line++, column, -SMALL_W, "금일:%6.1f\r\n", get_rainfall()->rainfall_today);
  vt100_print_bar(line++, column, -SMALL_W, "10분:%6.1f\r\n", get_rainfall()->rainfall_10min);
  vt100_print_bar(line++, column, -SMALL_W, "시간:%6.1f\r\n", get_rainfall()->rainfall_hourly);
  vt100_print_bar(line++, column, -SMALL_W, "월간:%6.1f\r\n", get_rainfall()->rainfall_monthly);
  vt100_print_bar(line++, column, -SMALL_W, "연간:%6.1f\r\n", get_rainfall()->rainfall_yearly);


  vt100_print_bar(line++, column, -SMALL_W, "\r\n");
  vt100_print_line(line++, column, '+', '-', SMALL_W);

  return line - (row+1);
}



int32_t print_eth_info(uint16_t row, uint16_t column,  uint8_t selected)
{
  DATE_TIME_BUF nt;
  eLINK_STATUS_t link_status[ETH_CLIENT_MAX];
  uint8_t tx_cnt[ETH_CLIENT_MAX];
  uint8_t rx_cnt[ETH_CLIENT_MAX];
  uint8_t line = row + 4;
  uint32_t last_time;


  if (get_config_app()->eth_mode == eETH_MODE_CLINET)
  {
    link_status[ETH_CLIENT_0] = get_tcp_client_system()->link_status;
    tx_cnt[ETH_CLIENT_0] = get_tcp_client_system()->tx_cnt;
    rx_cnt[ETH_CLIENT_0] = get_tcp_client_system()->rx_cnt;

    vt100_print_frame_selected(row+1, column, "이더넷", '+', '|', '-', SMALL_W, WHITE, selected);
    vt100_print_bar(line++, column, -SMALL_W, "링크  :%s\r\n",
                    ITEM_LIST(link_status[ETH_CLIENT_0], linkStatusList));
    vt100_print_bar(line++, column, -SMALL_W, "송신  :%d\r\n", tx_cnt[ETH_CLIENT_0]);
    vt100_print_bar(line++, column, -SMALL_W, "수신  :%d\r\n", rx_cnt[ETH_CLIENT_0]);

    last_time = get_tcp_client_system()->last_recv_time;
    time_cvt_secTotime(last_time, &nt);
    if (last_time == 0)
    {
      vt100_print_bar(line++, column, -SMALL_W, "R시간  :-\r\n");
    }
    else
    {
      vt100_print_bar(line++, column, -SMALL_W, "R시간  :%02d-%02d-%02d %02d:%02d:%02d\r\n",
                      nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
    }

    last_time = get_tcp_client_system()->last_send_time;
    time_cvt_secTotime(last_time, &nt);
    if (last_time == 0)
    {
      vt100_print_bar(line++, column, -SMALL_W, "T시간  :-\r\n");
    }
    else
    {
      vt100_print_bar(line++, column, -SMALL_W, "T시간  :%02d-%02d-%02d %02d:%02d:%02d\r\n",
                      nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
    }

    vt100_print_line(line++, column, '+', '-', SMALL_W);
  }
  else
  {
    vt100_print_frame_selected(row+1, column, "이더넷", '+', '|', '-', SMALL_W, WHITE, selected);

    link_status[ETH_CLIENT_0] = get_tcp_system(ETH_CLIENT_0)->link_status;
    tx_cnt[ETH_CLIENT_0] = get_tcp_system(ETH_CLIENT_0)->tx_cnt;
    rx_cnt[ETH_CLIENT_0] = get_tcp_system(ETH_CLIENT_0)->rx_cnt;

    vt100_print_bar(line++, column, -SMALL_W, "링크(0):%s(%s)\r\n",
                    ITEM_LIST(link_status[ETH_CLIENT_0], linkStatusList),
                    get_tcp_system(ETH_CLIENT_0)->client_ip_str);
    vt100_print_bar(line++, column, -SMALL_W, "송신   :%d\r\n", tx_cnt[ETH_CLIENT_0]);
    vt100_print_bar(line++, column, -SMALL_W, "수신   :%d\r\n", rx_cnt[ETH_CLIENT_0]);

    last_time = get_tcp_system(ETH_CLIENT_0)->last_recv_time;
    time_cvt_secTotime(last_time, &nt);
    if (last_time == 0)
    {
      vt100_print_bar(line++, column, -SMALL_W, "R시간  :-\r\n");
    }
    else
    {
      vt100_print_bar(line++, column, -SMALL_W, "R시간  :%02d-%02d-%02d %02d:%02d:%02d\r\n",
                      nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
    }

    last_time = get_tcp_system(ETH_CLIENT_0)->last_send_time;
    time_cvt_secTotime(last_time, &nt);
    if (last_time == 0)
    {
      vt100_print_bar(line++, column, -SMALL_W, "T시간  :-\r\n");
    }
    else
    {
      vt100_print_bar(line++, column, -SMALL_W, "T시간  :%02d-%02d-%02d %02d:%02d:%02d\r\n",
                      nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
    }



    link_status[ETH_CLIENT_1] = get_tcp_system(ETH_CLIENT_1)->link_status;
    tx_cnt[ETH_CLIENT_1] = get_tcp_system(ETH_CLIENT_1)->tx_cnt;
    rx_cnt[ETH_CLIENT_1] = get_tcp_system(ETH_CLIENT_1)->rx_cnt;

    vt100_print_bar(line++, column, -SMALL_W, "링크(1):%s(%s)\r\n",
                    ITEM_LIST(link_status[ETH_CLIENT_1], linkStatusList),
                    get_tcp_system(ETH_CLIENT_1)->client_ip_str);
    vt100_print_bar(line++, column, -SMALL_W, "송신   :%d\r\n", tx_cnt[ETH_CLIENT_1]);
    vt100_print_bar(line++, column, -SMALL_W, "수신   :%d\r\n", rx_cnt[ETH_CLIENT_1]);
    last_time = get_tcp_system(ETH_CLIENT_1)->last_recv_time;
    time_cvt_secTotime(last_time, &nt);
    if (last_time == 0)
    {
      vt100_print_bar(line++, column, -SMALL_W, "R시간  :-\r\n");
    }
    else
    {
      vt100_print_bar(line++, column, -SMALL_W, "R시간  :%02d-%02d-%02d %02d:%02d:%02d\r\n",
                      nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
    }

    last_time = get_tcp_system(ETH_CLIENT_1)->last_send_time;
    time_cvt_secTotime(last_time, &nt);
    if (last_time == 0)
    {
      vt100_print_bar(line++, column, -SMALL_W, "T시간  :-\r\n");
    }
    else
    {
      vt100_print_bar(line++, column, -SMALL_W, "T시간  :%02d-%02d-%02d %02d:%02d:%02d\r\n",
                      nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
    }
    vt100_print_line(line++, column, '+', '-', SMALL_W);
  }

  return line - (row+1);
}

uint8_t check_selected(uint8_t window_index, uint8_t pos,uint8_t selected)
{
  uint8_t selected_sum=0;
  
  if (window_index == pos)
  {
    selected_sum = 1;
    if (selected)
    {
      selected_sum = 2;
    }
  }

  return selected_sum;
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
int32_t print_cdma_info(uint16_t row, uint16_t column,  uint8_t selected)
{
  char buff[30];
  char num[20];
  uint8_t line = row + 4;
  DATE_TIME_BUF nt;
  uint32_t last_time;
  


    make_comList(buff, sizeof(buff));
    vt100_print_frame_selected(row+1, column, "CDMA", '+', '|', '-', SMALL_W, WHITE, selected);
    vt100_print_bar(line++, column, -SMALL_W, "링크    :%s\r\n",
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
  vt100_print_bar(line++, column, -SMALL_W, "전화번호:%s\r\n", num);
  if (get_cdma_system()->rssi == -1)
  {
    vt100_print_bar(line++, column, -SMALL_W, "수신감도:-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -SMALL_W, "수신감도:%d\r\n", get_cdma_system()->rssi);
  }

  vt100_print_bar(line++, column, -SMALL_W, "송신    :%d\r\n", get_cdma_system()->tx_cnt);
  vt100_print_bar(line++, column, -SMALL_W, "수신    :%d\r\n", get_cdma_system()->rx_cnt);

  last_time = get_cdma_system()->last_recv_time;
  time_cvt_secTotime(last_time, &nt);
  if(last_time==0)
  {
    vt100_print_bar(line++, column, -SMALL_W, "R시간   :-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -SMALL_W, "R시간   :%02d-%02d-%02d %02d:%02d:%02d\r\n",
                    nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  last_time = get_cdma_system()->last_send_time;
  time_cvt_secTotime(last_time, &nt);
  if(last_time==0)
  {
    vt100_print_bar(line++, column, -SMALL_W, "T시간   :-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -SMALL_W, "T시간   :%02d-%02d-%02d %02d:%02d:%02d\r\n",
                    nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  vt100_print_line(line++, column, '+', '-', SMALL_W);

  return line - (row+1);
}

int32_t print_direct_info(uint16_t row, uint16_t column, uint8_t selected)
{
  char buffer[30];

  uint8_t line = row + 4;

  DATE_TIME_BUF nt;
  uint32_t last_time;

  uint32_t remain_sec;

  remain_sec = (uint32_t)(get_direct_system()->linkdown_remain_ms/1000.0);
   make_comList(buffer, sizeof(buffer));
   vt100_print_frame_selected(row+1, column, "DIRECT", '+', '|', '-', SMALL_W, WHITE, selected);
   vt100_print_bar(line++, column, -SMALL_W, "링크    :%s\r\n",
                   ITEM_LIST(get_direct_system()->link_status, linkStatusList));
   vt100_print_bar(line++, column, -SMALL_W, "타임아웃:%ds\r\n", remain_sec);
   vt100_print_bar(line++, column, -SMALL_W, "송신    :%d\r\n", get_direct_system()->tx_cnt);
   vt100_print_bar(line++, column, -SMALL_W, "수신    :%d\r\n", get_direct_system()->rx_cnt);

   last_time = get_direct_system()->last_recv_time;

   if (last_time == 0)
   {
     vt100_print_bar(line++, column, -SMALL_W, "R시간   :-\r\n");
   }
  else
  {
    time_cvt_secTotime(last_time, &nt);

    vt100_print_bar(line++, column, -SMALL_W, "R시간   :%02d-%02d-%2d %02d:%02d:%02d\r\n",
                    nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  last_time = get_direct_system()->last_send_time;

  if(last_time == 0 )
  {
    time_cvt_secTotime(last_time, &nt);
    vt100_print_bar(line++, column, -SMALL_W, "T시간   :-\r\n");
  }
  else
  {
    time_cvt_secTotime(last_time, &nt);
    vt100_print_bar(line++, column, -SMALL_W, "T시간   :%02d-%02d-%2d %02d:%02d:%02d\r\n",
                    nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  vt100_print_bar(line++, column, -SMALL_W, " \r\n");;
  vt100_print_line(line++, column, '+', '-', SMALL_W);

  return line - (row+1);
}

//[AWS = (관측값+100)/10, 관측값 = (x-1000)/10]
#define KMA_TO_TEMPERATURE(x) ((float)((x - 1000) / 10.0f))  
#define KMA_TO_GENERAL(x) ((float)(x / 10.0f))
#define KMA_TO_1000(x) ((float)((x - 1000) / 10.0f))
#define KMA_TO_ILLUMINANCE(x) ((x) / 100.0f)
#define KMA_TO_RADI(x) ((x) / 10.0f - 100.0f)

#define COL_WIDTH 15

#define AWS_W 61
int32_t print_aws_info(uint16_t row, uint16_t column, eAWS_DATA_MIN_t min, uint8_t selected,
                       keycode_t key)
{
  const char *aws_title_list[] = {"순간(평균)", "1분", "10분", "한시간", "RAW"};
  char err_buf[32];
  char buff[60];
  uint8_t err;
  uint8_t line = row + 4;
  kma_data_ex_t *p_kma = NULL;
  float data;
  float data_min;
  float data_max;
  p_kma = get_kma_data(min);

  snprintf(buff, sizeof(buff), "AWS %s %.2fs/%.2fs [페이지 전환 LEFT,RIGHT 키 사용]", aws_title_list[min],
           (float)g_exec_250ms_time.elapsed_time / 1000.0f,
           (float)g_exec_1s_time.elapsed_time / 1000.0f);

  vt100_print_frame_selected(row + 1, column, buff, '+', '|', '-', AWS_W, WHITE, selected);

  // 온도
  if (p_kma->temperature.enable)
  {
    err = p_kma->temperature.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "기온", err_buf);
    }
    else
    {
      if(min == eAWS_DATA_RAW)
      {
        data = KMA_TO_TEMPERATURE(p_kma->temperature.data);
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH,"기온", data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->temperature.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->temperature.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->temperature.max);
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C  ,최소:%7.2f C  ,최대:%7.2f C\r\n",
                      COL_WIDTH, "기온", data, data_min, data_max);
      }
    }
  }

  if (p_kma->wind_direction_avg.enable)
  {
    err = p_kma->wind_direction_avg.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "풍향", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        data = KMA_TO_GENERAL(p_kma->wind_direction_avg.data);
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f 도\r\n",COL_WIDTH, "풍향", data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->wind_direction_avg.data);
        data_max = KMA_TO_GENERAL(p_kma->wind_direction_avg.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f 도 ,최대:%7.2f 도\r\n",
                        COL_WIDTH, "풍향", data, data_max);
      }
    }
  }

  if (p_kma->wind_speed_avg.enable)
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "풍속", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        data = KMA_TO_GENERAL(p_kma->wind_speed_avg.data);
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m/s\r\n",COL_WIDTH, "풍속", data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->wind_speed_avg.data);
        data_max = KMA_TO_GENERAL(p_kma->wind_speed_avg.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m/s,최대:%7.2f m/s\r\n",
                        COL_WIDTH, "풍속", data,  data_max);
      }
    }
  }

  if (p_kma->wind_direction_avg.enable &&
      (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR && min != eAWS_DATA_RAW))
  {
    err = p_kma->wind_direction_avg.err;
    if (err)
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "순간 풍향");
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "순간 풍향");
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f 도\r\n", COL_WIDTH, "순간 풍향",
                        KMA_TO_GENERAL(p_kma->wind_direction_instant.data));
      }
    }
  }

  if (p_kma->wind_speed_avg.enable &&
      (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR && min != eAWS_DATA_RAW))
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "순간 풍속");
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "순간 풍속");
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m/s\r\n", COL_WIDTH, "순간 풍속",
                        KMA_TO_GENERAL(p_kma->wind_speed_instant.data));
      }
    }
  }

  if (p_kma->precipitation.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "강수량", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        uint32_t last_time;
        last_time = p_kma->precipitation.last_time;
        DATE_TIME_BUF nt;

        if (last_time == 0)  // 우량이 내린적이 없으면
        {
          vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "강수량(time)");
        }
        else
        {
          time_cvt_secTotime(last_time, &nt);
          vt100_print_bar(line++, column, -AWS_W, "%-*s:%04d-%02d-%02d %02d:%02d:%02d\r\n", COL_WIDTH,
                          "강수량(time)", nt.Year, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
        }
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f mm\r\n", COL_WIDTH, "강수량",
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
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "기압", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        data = KMA_TO_GENERAL(p_kma->pressure.data);
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2fhPa\r\n", COL_WIDTH, "기압", data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->pressure.data);
        data_min = KMA_TO_GENERAL(p_kma->pressure.min);
        data_max = KMA_TO_GENERAL(p_kma->pressure.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f hPa,최소:%7.2f hPa,최대:%7.2f hPa\r\n",
                        COL_WIDTH, "기압", data, data_min, data_max);
      }
    }
  }

  if (p_kma->precipitation_presence.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation_presence.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "강수유무", err_buf);
    }
    else
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%5d \r\n", COL_WIDTH, "강수유무",
                      p_kma->precipitation_presence.data);
    }
  }

  if (p_kma->snowfall.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->snowfall.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "적설", err_buf);
    }
    else
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%7d mm\r\n", COL_WIDTH, "적설",
                      p_kma->snowfall.data);
    }
  }

  if (p_kma->relative_humidity.enable)
  {
    err = p_kma->relative_humidity.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "상대습도", err_buf);
    }
    else
    {
      data = KMA_TO_GENERAL(p_kma->relative_humidity.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f %%\r\n", COL_WIDTH,"상대습도", data);
      }
      else
      {
        data_min = KMA_TO_GENERAL(p_kma->relative_humidity.min);
        data_max = KMA_TO_GENERAL(p_kma->relative_humidity.max);

      vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f %%,최소:%7.2f %%,최대:%7.2f %%\r\n", COL_WIDTH,
                      "상대습도",data,data_min,data_max);
      }
    }
  }

  if (p_kma->precipitation_fine.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation_fine.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "강수량(0.1)", err_buf);
    }
    else
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f mm\r\n", COL_WIDTH, "강수량(0.1)",
                      KMA_TO_GENERAL(p_kma->precipitation_fine.data));
    }
  }

  if (p_kma->solar_radiation.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->solar_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "일사", err_buf);
    }
    else
    {
      float solar_radiation;

      solar_radiation = p_kma->solar_radiation.data;

      switch (min)
      {
        case eAWS_DATA_RAW:
        case eAWS_DATA_AVG:
          vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f WJ/m2\r\n", COL_WIDTH, "일사",
                          solar_radiation);
          break;

        default:
          vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f KJ/m2\r\n", COL_WIDTH, "일사",
                          solar_radiation);
          break;
      }
    }
  }

  if (p_kma->sunshine_duration.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->sunshine_duration.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "일조", err_buf);
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
          vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "일조",
                          sunshine_duration ? "ON" : "OFF");
        }
        break;

        default:
          vt100_print_bar(line++, column, -AWS_W, "%-*s:%5d s\r\n", COL_WIDTH, "일조",
                          p_kma->sunshine_duration.data);
          break;
      }
    }
  }

  if (p_kma->surface_temperature.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->surface_temperature.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지면온도", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW || min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "지면온도",
                        KMA_TO_TEMPERATURE(p_kma->surface_temperature.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "지면온도");
      }
    }
  }

  if (p_kma->grass_temperature.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->grass_temperature.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "초상온도", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW || min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "초상온도",
                        KMA_TO_TEMPERATURE(p_kma->grass_temperature.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:-\r\n", COL_WIDTH, "초상온도");
      }
    }
  }

  if (p_kma->soil_temperature_5cm.enable)
  {
    err = p_kma->soil_temperature_5cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지중온도 5cm", err_buf);
    }
    else
    {
      data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2fC\r\n", COL_WIDTH,"지중온도 5cm", data);
      }
      else
      {
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C,최소:%7.2f C,최대:%7.2f C\r\n",
                        COL_WIDTH, "지중온도 5cm", data, data_min, data_max);
      }
    }
  }

  if (p_kma->soil_temperature_10cm.enable)
  {
    err = p_kma->soil_temperature_10cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지중온도 10cm", err_buf);
    }
    else
    {
      data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "지중온도 10cm", data);
      }
      else
      {
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C,최소:%7.2f C,최대:%7.2f C\r\n",
                        COL_WIDTH, "지중온도 10cm", data, data_min, data_max);
      }
    }
  }

  if (p_kma->soil_temperature_20cm.enable)
  {
    err = p_kma->soil_temperature_20cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지중온도 20cm", err_buf);
    }
    else
    {
      data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "지중온도 20cm",
                        data);
      }
      else
      {
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C,최소:%7.2f C,최대:%7.2f C\r\n",
                        COL_WIDTH, "지중온도 20cm", data, data_min, data_max);
      }
    }
  }

  if (p_kma->soil_temperature_30cm.enable)
  {
    err = p_kma->soil_temperature_30cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지중온도 30cm", err_buf);
    }
    else
    {
      data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "지중온도 30cm",
                        data);
      }
      else
      {
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C,최소:%7.2f C,최대:%7.2f C\r\n",
                        COL_WIDTH, "지중온도 30cm", data, data_min, data_max);
      }
    }
  }

  if (p_kma->soil_temperature_50cm.enable)
  {
    err = p_kma->soil_temperature_50cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지중온도 50cm", err_buf);
    }
    else
    {
      data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "지중온도 50cm",
                        data);
      }
      else
      {
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C,최소:%7.2f C,최대:%7.2f C\r\n",
                        COL_WIDTH, "지중온도 50cm", data, data_min, data_max);
      }
    }
  }

  if (p_kma->soil_temperature_1m.enable)
  {
    err = p_kma->soil_temperature_1m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지중온도 1m", err_buf);
    }
    else
    {
      data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "지중온도 1m",
                        data);
      }
      else
      {
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C,최소:%7.2f C,최대:%7.2f C\r\n",
                        COL_WIDTH, "지중온도 1m", data, data_min, data_max);
      }
    }
  }

  if (p_kma->soil_temperature_1_5m.enable)
  {
    err = p_kma->soil_temperature_1_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지중온도 1.5m", err_buf);
    }
    else
    {
      data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "지중온도 1.5m",
                        data);
      }
      else
      {
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C,최소:%7.2f C,최대:%7.2f C\r\n",
                        COL_WIDTH, "지중온도 1.5m", data, data_min, data_max);
      }
    }
  }

  if (p_kma->soil_temperature_3m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_temperature_3m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지중온도 3m", err_buf);
    }
    else
    {
      data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "지중온도 3m",
                        data);
      }
      else
      {
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C,최소:%7.2f C,최대:%7.2f C\r\n",
                        COL_WIDTH, "지중온도 3m", data, data_min, data_max);
      }
    }
  }

  if (p_kma->soil_temperature_5m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_temperature_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "지중온도 5m", err_buf);
    }
    else
    {
      data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.data);
      if (min == eAWS_DATA_RAW)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "지중온도 5m",
                        data);
      }
      else
      {
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.max);

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C,최소:%7.2f C,최대:%7.2f C\r\n",
                        COL_WIDTH, "지중온도 5m", data, data_min, data_max);
      }
    }
  }

  if (p_kma->cloud_height_1st.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->cloud_height_1st.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "운고 1층", err_buf);
    }
    else
    {

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m\r\n", COL_WIDTH, "운고 1층",
                        KMA_TO_GENERAL(p_kma->cloud_height_1st.data));
 
    }
  }

  if (p_kma->cloud_height_2nd.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->cloud_height_2nd.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "운고 2층", err_buf);
    }
    else
    {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m\r\n", COL_WIDTH, "운고 2층",
                        KMA_TO_GENERAL(p_kma->cloud_height_2nd.data));
    }
  }

  if (p_kma->cloud_height_3rd.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->cloud_height_3rd.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "운고 3층", err_buf);
    }
    else
    {

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m\r\n", COL_WIDTH, "운고 3층",
                        KMA_TO_GENERAL(p_kma->cloud_height_3rd.data));

    }
  }

  if (p_kma->cloud_amount.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->cloud_amount.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "운량", err_buf);
    }
    else
    {

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f \r\n", COL_WIDTH, "운량",
                        KMA_TO_GENERAL(p_kma->cloud_amount.data));

    }
  }

  if (p_kma->visibility.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->visibility.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "시정", err_buf);
    }
    else
    {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m\r\n", COL_WIDTH, "시정",
                        KMA_TO_GENERAL(p_kma->visibility.data));

    }
  }

  if (p_kma->pm10_concentration.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->pm10_concentration.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "미세먼지 PM10", err_buf);
    }
    else
    {

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f ug/m3\r\n", COL_WIDTH, "미세먼지 PM10",
                        KMA_TO_GENERAL(p_kma->pm10_concentration.data));

    }
  }

  if (p_kma->pm25_concentration.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->pm25_concentration.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "미세먼지 PM2.5", err_buf);
    }
    else
    {

        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f ug/m3\r\n", COL_WIDTH, "미세먼지 PM2.5",
                        KMA_TO_GENERAL(p_kma->pm25_concentration.data));

    }
  }

  if (p_kma->net_radiation.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->net_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "순복사", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG || min == eAWS_DATA_1MIN)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f W/m2\r\n", COL_WIDTH, "순복사",
                        KMA_TO_RADI(p_kma->net_radiation.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:-\r\n", COL_WIDTH, "순복사");
      }
    }
  }

  if (p_kma->total_radiation.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->total_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "전천복사", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f W/m2\r\n", COL_WIDTH, "전천복사",
                        KMA_TO_RADI(p_kma->total_radiation.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f W/m2");
      }
    }
  }

  if (p_kma->reflected_radiation.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->reflected_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "반사복사", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f W/m2\r\n", COL_WIDTH, "반사복사",
                        KMA_TO_RADI(p_kma->reflected_radiation.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f W/m2");
      }
    }
  }

  if (p_kma->direct_radiation.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->direct_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "직달일사", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f W/m2\r\n", COL_WIDTH, "직달일사",
                        KMA_TO_RADI(p_kma->direct_radiation.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f W/m2");
      }
    }
  }

  if (p_kma->current_weather.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->current_weather.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "현재일기", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f \r\n", COL_WIDTH, "현재일기",
                        KMA_TO_GENERAL(p_kma->current_weather.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f");
      }
    }
  }

  if (p_kma->soil_moisture_10cm.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_moisture_10cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "토양수분 10cm", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f %\r\n", COL_WIDTH, "토양수분 10cm",
                        KMA_TO_GENERAL(p_kma->soil_moisture_10cm.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f %");
      }
    }
  }

  if (p_kma->soil_moisture_20cm.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_moisture_20cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "토양수분 20cm", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f %%\r\n", COL_WIDTH, "토양수분 20cm",
                        KMA_TO_GENERAL(p_kma->soil_moisture_20cm.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f %%");
      }
    }
  }

  if (p_kma->soil_moisture_30cm.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_moisture_30cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "토양수분 30cm", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f %%\r\n", COL_WIDTH, "토양수분 30cm",
                        KMA_TO_GENERAL(p_kma->soil_moisture_30cm.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f %%");
      }
    }
  }

  if (p_kma->soil_moisture_50cm.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_moisture_50cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "토양수분 50cm", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f %%\r\n", COL_WIDTH, "토양수분 50cm",
                        KMA_TO_GENERAL(p_kma->soil_moisture_50cm.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f %%");
      }
    }
  }

  if (p_kma->illuminance.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->illuminance.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "조도", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f klux\r\n", COL_WIDTH, "조도",
                        KMA_TO_ILLUMINANCE(p_kma->illuminance.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f klux");
      }
    }
  }

  if (p_kma->wind_speed_1_5m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->wind_speed_1_5m.err;
    if (err)
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "풍속 1.5m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m/s\r\n", COL_WIDTH, "풍속 1.5m",
                        KMA_TO_GENERAL(p_kma->wind_speed_1_5m.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f m/s");
      }
    }
  }

  if (p_kma->wind_speed_4m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->wind_speed_4m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "풍속 4.0m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m/s\r\n", COL_WIDTH, "풍속 4.0m",
                        KMA_TO_GENERAL(p_kma->wind_speed_4m.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f m/s");
      }
    }
  }

  if (p_kma->instant_wind_speed_1_5m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->instant_wind_speed_1_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "순간풍속 1.5m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m/s\r\n", COL_WIDTH, "순간풍속 1.5m",
                        KMA_TO_GENERAL(p_kma->instant_wind_speed_1_5m.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f m/s");
      }
    }
  }

  if (p_kma->instant_wind_speed_4m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->instant_wind_speed_4m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "순간풍속 4.0m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f m/s\r\n", COL_WIDTH, "순간풍속 4.0m",
                        KMA_TO_GENERAL(p_kma->instant_wind_speed_4m.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f m/s");
      }
    }
  }

  if (p_kma->temperature_0_5m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->temperature_0_5m.err;
    if (err)
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "기온 0.5m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "기온 0.5m",
                        KMA_TO_TEMPERATURE(p_kma->temperature_0_5m.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f C");
      }
    }
  }

  if (p_kma->temperature_4m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->temperature_4m.err;
    if (err)
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "기온 4.0m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f C\r\n", COL_WIDTH, "기온 4.0m",
                        KMA_TO_TEMPERATURE(p_kma->temperature_4m.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "%7.2f C");
      }
    }
  }

  if (p_kma->humidity_0_5m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->humidity_0_5m.err;
    if (err)
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "습도 0.5m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f %%\r\n", COL_WIDTH, "습도 0.5m",
                        KMA_TO_GENERAL(p_kma->humidity_0_5m.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "습도 0.5m");
      }
    }
  }

  if (p_kma->humidity_4m.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->humidity_4m.err;
    if (err)
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "습도 4.0m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f %%\r\n", COL_WIDTH, "습도 4.0m",
                        KMA_TO_GENERAL(p_kma->humidity_4m.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "습도 4.0m");
      }
    }
  }

  if (p_kma->tacometer.enable && (min != eAWS_DATA_10MIN  && min != eAWS_DATA_HOUR))
  {
    err = p_kma->tacometer.err;
    if (err)
    {
      vt100_print_bar(line++, column, -AWS_W, "%-*s:%s\r\n", COL_WIDTH, "타코미터", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_AVG)
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:%7.2f rpm\r\n", COL_WIDTH, "타코미터",
                        KMA_TO_GENERAL(p_kma->tacometer.data));
      }
      else
      {
        vt100_print_bar(line++, column, -AWS_W, "%-*s:--\r\n", COL_WIDTH, "타코미터");
      }
    }
  }

  vt100_print_line(line++, column, '+', '-', AWS_W);

  return line;
}


#define CENTER_OFFSET 2
#define RIGHT_OFFSET 1



int32_t aws_menu_display(void)
{
  uint8_t awsMode = 0;
  uint8_t selected = 1;
  uint8_t window_index = 1;  // 창 고유 번호
  uint8_t window_pos = 1;    // 커서가 현재 어떤창을 가리키는지
  uint8_t window_selected = 0;
  keycode_t key;
  int32_t row = 0;




  io_printf(VT100_CLEAR_SCREEN);
  io_printf(VT100_CURSOR_OFF);




  do
  {
    io_printf(VT100_CURSOR_HOME);
    io_printf("\r\n");

    window_index=0;
    row = 0;

    row = print_rain_info(row, 0, check_selected(window_index++, window_pos, selected));

    window_selected = check_selected(window_index++, window_pos, selected);
    row = print_aws_info(row, 0, (eAWS_DATA_MIN_t)awsMode, window_selected, key);

    row = 0;

    row = print_system_info(0, (SMALL_W + CENTER_OFFSET) + RIGHT_OFFSET,
                            check_selected(window_index++, window_pos, selected));

    row = 0;

    row = print_charger_info(row, (SMALL_W + CENTER_OFFSET) * 2 + RIGHT_OFFSET,
                             check_selected(window_index++, window_pos, selected));

    row = 0;
    if (get_config_app()->cdma_use)
    {

      row = print_cdma_info(0, (SMALL_W + CENTER_OFFSET) * 3 + RIGHT_OFFSET,
                            check_selected(window_index++, window_pos, selected));
    }
    if (get_config_app()->direct_use)
    {
      row += print_direct_info(row, (SMALL_W + CENTER_OFFSET) * 3 + RIGHT_OFFSET,
                               check_selected(window_index++, window_pos, selected));
    }
    if (get_config_app()->eth_use)
    {

      row += print_eth_info(row, (SMALL_W + CENTER_OFFSET) * 3 + RIGHT_OFFSET,
                            check_selected(window_index++, window_pos, selected));
    }



    key = (keycode_t)get_key(500);
    if (s_navi)
    {
      if (key == KEY_CODE_RIGHT)
      {
        if (window_pos < (window_index - 1))
          window_pos++;
        io_printf(VT100_CLEAR_SCREEN);
      }
      else if (key == KEY_CODE_LEFT)
      {
        if (window_pos > 0)
        {
          window_pos--;
        }
        io_printf(VT100_CLEAR_SCREEN);
      }
      else if (key == KEY_CODE_ENTER)
      {
        s_navi = 0;
        selected = 1;
      }
    }
    else
    {
      if(key == KEY_CODE_RIGHT)
      {
        io_printf(VT100_CLEAR_SCREEN);
        if (window_selected)
        {
          if (awsMode < eAWS_DATA_RAW)
          {
            awsMode++;
          }
        }
      }
      else if(key == KEY_CODE_LEFT)
      {
        io_printf(VT100_CLEAR_SCREEN);
        if(window_selected)
        {
          if (awsMode > 0)
        {
          awsMode--;
        }
      }
      }
      else if (key ==KEY_CODE_ENTER)
      {
        selected =0;
        s_navi =1;
      }
      
    }

    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
  }
      while (1);

  vt100_print(50, 0, "\r\n");
  io_printf(VT100_CURSOR_ON);
  return 0;
}