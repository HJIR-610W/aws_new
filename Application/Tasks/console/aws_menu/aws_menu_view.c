

#include "aws_menu_view.h"

#include <string.h>
#include "view_driver.h"

#include "util_time.h"

#include "config_app.h"
#include "console_utile.h"
#include "task_logging.h"
#include "bsp_di.h"
#include "config_nvm.h"
#include "aws_data.h"
#include "dev_io.h"
#include "task_system.h"
#include "bsp.h"
#include "app_charger.h"
#include "task_direct.h"
#include "task_cellular.h"
#include "task_tcpServer.h"
#include "task_client.h"
#include "task_measure.h"
#include "vt100_command.h"
#include "util_stdio.h"
const char *linkStatusList[3] = {"-", "UP", "DOWN"};
const char *doorStatusList[2] = {"닫힘", "열림"};
const char *generalStatusList[2] = {"정상", "비정상"};

//utf8용 자간 일정하게 만드는 make_label
char *m_l(char *label,int width)
{
  int len;
  int remain;
  static char buff[15];

  strcpy_safe(buff,sizeof(buff),label);
  len = strlen(buff);

  remain = width - utf8_strlen(label);

  for (int i = 0; i < remain; i++)
  {
    buff[len++] = ' ';
  }
  buff[len]=0;
  
  return buff;

}
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
    snprintf(buffer, buffer_size, "E(V%u,C%u)", val_err, comm_err);
  }
  else if (val_err)
  {
    snprintf(buffer, buffer_size, "E(V%u)", val_err);
  }
  else if (comm_err)
  {
    snprintf(buffer, buffer_size, "E(C%u)", comm_err);
  }
}



void reset_win(win_t *win)
{
  win->is_selected = 0;
  win->is_focused = 0;
}

#define SYSTEM_WD 10
void draw_system(win_t* p_win)
{
  int row_count = 0;
  char buff[50];
  int page=0;
  int win_height = p_win->view_row + 3;
  const char *message;
   p_win->total_pages = 1;
  p_win->current_row = 0;
  calculate_window_position(p_win, p_win->view_col, win_height);


  switch (page)
  {
    case 0:
    {
      win_printf_title(p_win, "시스템");

      snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year,
               Date_Time.Month, Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
      win_printf_row(p_win, row_count++, buff);

      snprintf(buff, sizeof(buff), "%s: %d", m_l("ID",SYSTEM_WD), get_config_app()->id);
      win_printf_row(p_win, row_count++, buff);

      snprintf(buff, sizeof(buff), "%s: %s", m_l("문 상태",SYSTEM_WD),ITEM_LIST(IS_DOOR_OPENED(), doorStatusList));
      win_printf_row(p_win, row_count++, buff);

      if (get_logging_system()->status_group)
      {
        message = "오류";
      }
      else
      {
        message = "정상";
      }

      snprintf(buff, sizeof(buff), "%s: %s", m_l("저장 기능",SYSTEM_WD), message);
      win_printf_row(p_win, row_count++, buff);

      snprintf(buff, sizeof(buff), "%s: %.1f", m_l("장비 전원V", SYSTEM_WD),
               bsp_read_battery());
      win_printf_row(p_win, row_count++, buff);

      snprintf(buff, sizeof(buff), "%s: %.1f", m_l("장비 온도C",SYSTEM_WD), bsp_read_temperature());
      win_printf_row(p_win, row_count++, buff);

      if (get_config_app()->ac_use)
      {
        snprintf(buff, sizeof(buff), "%s: %s",  m_l("AC",SYSTEM_WD),"정상");
        win_printf_row(p_win, row_count++,  buff);
      }

      p_win->total_items[page] = ALIGN_UP(row_count, p_win->view_row);

      break;
    }
    default:

      return;
  }

  // Fill remaining rows with blank lines to maintain consistent window size
  while (p_win->current_row < p_win->view_row)
  {
    win_printf_row(p_win, row_count++, "");
  }

  win_print_close(p_win);
}

#define RAIN_WD 8
void draw_rain(win_t *p_win)
{
  int row_count = 0;
  char buff[50];
  int page = 0;
  int win_height = p_win->view_row + 3;

  p_win->total_pages = 1;
  p_win->current_row = 0;
  calculate_window_position(p_win, p_win->view_col, win_height);

  switch (page)
  {
    case 0:
  {
    win_printf_title(p_win, "강수량");

    snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "전일", get_rainfall()->rainfall_yesterday);
    win_printf_row(p_win, row_count++, buff);

    snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "금일", get_rainfall()->rainfall_today);
    win_printf_row(p_win, row_count++, buff);

    snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "1분", get_rainfall()->rainfall_1min);
    win_printf_row(p_win, row_count++, buff);

    snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "10분", get_rainfall()->rainfall_10min);
    win_printf_row(p_win, row_count++, buff);

    snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "시간", get_rainfall()->rainfall_hourly);
    win_printf_row(p_win, row_count++, buff);

    snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "연간",  get_rainfall()->rainfall_yearly);
    win_printf_row(p_win, row_count++, buff);


    snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "월간", get_rainfall()->rainfall_monthly);
    win_printf_row(p_win, row_count++, buff);

    
    p_win->total_items[page] = ALIGN_UP(row_count, p_win->view_row);

      break;
    }
    default:

      return;
  }

  // Fill remaining rows with blank lines to maintain consistent window size
  while (p_win->current_row < p_win->view_row)
  {
    win_printf_row(p_win, row_count++, "");
  }

  win_print_close(p_win);
}

#define CHARGER_WD 15
void draw_charger(win_t *p_win)
{
  uint8_t err;
  int row_count = 0;
  char buff[50];
  char temp[20];
  int page = 0;
  int win_height = p_win->view_row + 3;

  p_win->total_pages = 1;
  p_win->current_row = 0;
  calculate_window_position(p_win, p_win->view_col, win_height);

  switch (page)
  {
    case 0:
    {
      win_printf_title(p_win, "충전기");
      read_chargerStatus(temp, sizeof(temp));

      snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "상태", temp);
      win_printf_row(p_win, row_count++, buff);

      if (is_chargerValid())
      {
        snprintf(buff, sizeof(buff), "%-*s: %.2f", CHARGER_WD, "충전전압(V)",
                 read_solarVoltage1(&err));
        win_printf_row(p_win, row_count++, buff);

        snprintf(buff, sizeof(buff), "%-*s: %.2f", CHARGER_WD, "충전전류(A)",
                 read_solarCurrrent1(&err));
        win_printf_row(p_win, row_count++, buff);

        snprintf(buff, sizeof(buff), "%-*s: %.2f", CHARGER_WD, "배터리 전압(V)",
                 read_batteryVoltage1(&err));
        win_printf_row(p_win, row_count++, buff);

        snprintf(buff, sizeof(buff), "%-*s: %.2f", CHARGER_WD, "부하 1 전류(A)",
                 read_loadCurrent1(&err));
        win_printf_row(p_win, row_count++, buff);

      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "충전전압(V)", "-");
        win_printf_row(p_win, row_count++, buff);

        snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "충전전류(A)", "-");
        win_printf_row(p_win, row_count++, buff);

        snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "배터리 전압(V)", "-");
        win_printf_row(p_win, row_count++, buff);

        snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "부하 1 전류(A)","-");
        win_printf_row(p_win, row_count++, buff);
      }

      p_win->total_items[page] = ALIGN_UP(row_count, p_win->view_row);

      break;
    }
    default:

      return;
  }

  // Fill remaining rows with blank lines to maintain consistent window size
  while (p_win->current_row < p_win->view_row)
  {
    win_printf_row(p_win, row_count++, "");
  }

  win_print_close(p_win);
}
#define DIRECT_WD 8


void draw_direct(win_t *p_win)
{

  int row_count = 0;
  char buff[50];

  int page = 0;
  int win_height = p_win->view_row + 3;

  DATE_TIME_BUF nt;
  uint32_t last_time;
  uint32_t remain_sec;

  p_win->total_pages = 1;
  p_win->current_row = 0;
  calculate_window_position(p_win, p_win->view_col, win_height);

  switch (page)
  {
    case 0:
    {
      win_printf_title(p_win, "직접통신");

      // 링크 상태
      snprintf(buff, sizeof(buff), "%s: %s", m_l("링크",DIRECT_WD),
               ITEM_LIST(get_direct_system()->link_status, linkStatusList));
      win_printf_row(p_win, row_count++, buff);

      // 타임아웃 (남은 시간)
      remain_sec = (uint32_t)(get_direct_system()->linkdown_remain_ms / 1000.0);
      snprintf(buff, sizeof(buff), "%s: %ds", m_l("타임아웃", DIRECT_WD), remain_sec);
      win_printf_row(p_win, row_count++, buff);

      // 송신 카운트
      snprintf(buff, sizeof(buff), "%s: %d", m_l("송신", DIRECT_WD), get_direct_system()->tx_cnt);
      win_printf_row(p_win, row_count++, buff);

      // 수신 카운트
      snprintf(buff, sizeof(buff), "%s: %d", m_l("수신", DIRECT_WD), get_direct_system()->rx_cnt);
      win_printf_row(p_win, row_count++, buff);

      // 마지막 수신 시간
      last_time = get_direct_system()->last_recv_time;
      if (last_time == 0)
      {
        snprintf(buff, sizeof(buff), "%s: -", m_l("R시간", DIRECT_WD));
      }
      else
      {
        time_cvt_secTotime(last_time, &nt);
        snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", DIRECT_WD, "R시간",
                 nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
      }
      win_printf_row(p_win, row_count++, buff);

      // 마지막 송신 시간
      last_time = get_direct_system()->last_send_time;
      if (last_time == 0)
      {
        snprintf(buff, sizeof(buff), "%-*s: -", DIRECT_WD, "T시간");
      }
      else
      {
        time_cvt_secTotime(last_time, &nt);
        snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", DIRECT_WD, "T시간",
                 nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
      }
      win_printf_row(p_win, row_count++, buff);

      p_win->total_items[page] = ALIGN_UP(row_count, p_win->view_row);

      break;
    }
    default:

      return;
  }

  // Fill remaining rows with blank lines to maintain consistent window size
  while (p_win->current_row < p_win->view_row)
  {
    win_printf_row(p_win, row_count++, "");
  }

  win_print_close(p_win);
}

#define CDMA_WD 15

void draw_cdma(win_t *p_win)
{

  int row_count = 0;
  char buff[50];
  char num[20];
  int page = 0;
  int win_height = p_win->view_row + 3;

  DATE_TIME_BUF nt;
  uint32_t last_time;

  p_win->total_pages = 1;
  p_win->current_row = 0;
  calculate_window_position(p_win, p_win->view_col, win_height);

  switch (page)
  {
    case 0:
    {
      win_printf_title(p_win, "CDMA");

      // 링크 상태
      snprintf(buff, sizeof(buff), "%-*s: %s", CDMA_WD, "링크",
               ITEM_LIST(get_cdma_system()->link_status, linkStatusList));
      win_printf_row(p_win, row_count++, buff);

      // 전화번호
      if (get_cdma_system()->num[0] != '0')
      {
        num[0] = '-';
        num[1] = 0;
      }
      else
      {
        snprintf(num, sizeof(num), "%s", get_cdma_system()->num);
      }
      snprintf(buff, sizeof(buff), "%-*s: %s", CDMA_WD, "전화번호", num);
      win_printf_row(p_win, row_count++, buff);

      // 수신감도
      if (get_cdma_system()->rssi == -1)
      {
        snprintf(buff, sizeof(buff), "%-*s: -", CDMA_WD, "수신감도");
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %d", CDMA_WD, "수신감도", get_cdma_system()->rssi);
      }
      win_printf_row(p_win, row_count++, buff);

      // 송신 카운트
      snprintf(buff, sizeof(buff), "%-*s: %d", CDMA_WD, "송신", get_cdma_system()->tx_cnt);
      win_printf_row(p_win, row_count++, buff);

      // 수신 카운트
      snprintf(buff, sizeof(buff), "%-*s: %d", CDMA_WD, "수신", get_cdma_system()->rx_cnt);
      win_printf_row(p_win, row_count++, buff);

      // 마지막 수신 시간
      last_time = get_cdma_system()->last_recv_time;
      if (last_time == 0)
      {
        snprintf(buff, sizeof(buff), "%-*s: -", CDMA_WD, "R시간");
      }
      else
      {
        time_cvt_secTotime(last_time, &nt);
        snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", CDMA_WD, "R시간",
                 nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
      }
      win_printf_row(p_win, row_count++, buff);

      // 마지막 송신 시간
      last_time = get_cdma_system()->last_send_time;
      if (last_time == 0)
      {
        snprintf(buff, sizeof(buff), "%-*s: -", CDMA_WD, "T시간");
      }
      else
      {
        time_cvt_secTotime(last_time, &nt);
        snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", CDMA_WD, "T시간",
                 nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
      }
      win_printf_row(p_win, row_count++, buff);

      p_win->total_items[page] = ALIGN_UP(row_count, p_win->view_row);

      break;
    }
    default:

      return;
  }

  // Fill remaining rows with blank lines to maintain consistent window size
  while (p_win->current_row < p_win->view_row)
  {
    win_printf_row(p_win, row_count++, "");
  }

  win_print_close(p_win);
}

#define ETH_WD 10

void draw_eth(win_t *p_win)
{

  int row_count = 0;
  char buff[50];
  int page = 0;
  int win_height = p_win->view_row + 3;

  DATE_TIME_BUF nt;
  eLINK_STATUS_t link_status[ETH_CLIENT_MAX];
  uint8_t tx_cnt[ETH_CLIENT_MAX];
  uint8_t rx_cnt[ETH_CLIENT_MAX];
  uint32_t last_time;

  p_win->total_pages = 1;
  p_win->current_row = 0;
  calculate_window_position(p_win, p_win->view_col, win_height);


      win_printf_title(p_win, "이더넷");

      if (get_config_app()->eth_mode == eETH_MODE_CLINET)
      {
        // 클라이언트 모드
        link_status[ETH_CLIENT_0] = get_tcp_client_system()->link_status;
        tx_cnt[ETH_CLIENT_0] = get_tcp_client_system()->tx_cnt;
        rx_cnt[ETH_CLIENT_0] = get_tcp_client_system()->rx_cnt;

        // 링크 상태
        snprintf(buff, sizeof(buff), "%-*s: %s", ETH_WD, "링크",
                 ITEM_LIST(link_status[ETH_CLIENT_0], linkStatusList));
        win_printf_row(p_win, row_count++, buff);

        // 송신 카운트
        snprintf(buff, sizeof(buff), "%-*s: %d", ETH_WD, "송신", tx_cnt[ETH_CLIENT_0]);
        win_printf_row(p_win, row_count++, buff);

        // 수신 카운트
        snprintf(buff, sizeof(buff), "%-*s: %d", ETH_WD, "수신", rx_cnt[ETH_CLIENT_0]);
        win_printf_row(p_win, row_count++, buff);

        // 마지막 수신 시간
        last_time = get_tcp_client_system()->last_recv_time;
        if (last_time == 0)
        {
          snprintf(buff, sizeof(buff), "%-*s: -", ETH_WD, "R시간");
        }
        else
        {
          time_cvt_secTotime(last_time, &nt);
          snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", ETH_WD, "R시간",
                   nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
        }
        win_printf_row(p_win, row_count++, buff);

        // 마지막 송신 시간
        last_time = get_tcp_client_system()->last_send_time;
        if (last_time == 0)
        {
          snprintf(buff, sizeof(buff), "%-*s: -", ETH_WD, "T시간");
        }
        else
        {
          time_cvt_secTotime(last_time, &nt);
          snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", ETH_WD, "T시간",
                   nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
        }
        win_printf_row(p_win, row_count++, buff);
      }
      else
      {
        // 서버 모드 - 여러 클라이언트 처리
        for (int i = 0; i < ETH_CLIENT_MAX; i++)
        {
          link_status[i] = get_tcp_system(i)->link_status;
          tx_cnt[i] = get_tcp_system(i)->tx_cnt;
          rx_cnt[i] = get_tcp_system(i)->rx_cnt;

          // 링크 상태 (클라이언트 번호와 IP 포함)
          snprintf(buff, sizeof(buff), "링크(%d): %s(%s)", i,
                   ITEM_LIST(link_status[i], linkStatusList), get_tcp_system(i)->client_ip_str);
          win_printf_row(p_win, row_count++, buff);

          // 송신 카운트
          snprintf(buff, sizeof(buff), "%-*s: %d", ETH_WD, "송신", tx_cnt[i]);
          win_printf_row(p_win, row_count++, buff);

          // 수신 카운트
          snprintf(buff, sizeof(buff), "%-*s: %d", ETH_WD, "수신", rx_cnt[i]);
          win_printf_row(p_win, row_count++, buff);

          // 마지막 수신 시간
          last_time = get_tcp_system(i)->last_recv_time;
          if (last_time == 0)
          {
            snprintf(buff, sizeof(buff), "%-*s: -", ETH_WD, "R시간");
          }
          else
          {
            time_cvt_secTotime(last_time, &nt);
            snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", ETH_WD, "R시간",
                     nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
          }
          win_printf_row(p_win, row_count++, buff);

          // 마지막 송신 시간
          last_time = get_tcp_system(i)->last_send_time;
          if (last_time == 0)
          {
            snprintf(buff, sizeof(buff), "%-*s: -", ETH_WD, "T시간");
          }
          else
          {
            time_cvt_secTotime(last_time, &nt);
            snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", ETH_WD, "T시간",
                     nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
          }
          win_printf_row(p_win, row_count++, buff);
        }
      }

      p_win->total_items[page] = ALIGN_UP(row_count, p_win->view_row);



  // Fill remaining rows with blank lines to maintain consistent window size
  while (p_win->current_row < p_win->view_row)
  {
    win_printf_row(p_win, row_count++, "");
  }

  win_print_close(p_win);
}

#define AWS_WD 15

void draw_aws(win_t *p_win)
{
  uint8_t err;
  int row_count = 0;
  char buff[80];
  char err_buf[32];
  int page = 0;
  int win_height = p_win->view_row + 3;

  kma_data_ex_t *p_kma = NULL;
  float data, data_min, data_max;

  const char *aws_title_list[] = {"순간(평균)", "1분", "10분", "한시간", "RAW"};
  p_win->current_row = 0;
  p_win->total_pages = 5;  // 0~4: 순간, 1분, 10분, 한시간, RAW
  calculate_window_position(p_win, p_win->view_col, win_height);

  page = p_win->current_page;
  p_kma = get_kma_data((eAWS_DATA_MIN_t)page);

  // 타이틀 설정
  snprintf(buff, sizeof(buff), "AWS %s %.2fs/%.2fs", aws_title_list[page],
           (float)g_exec_250ms_time.elapsed_time / 1000.0f,
           (float)g_exec_1s_time.elapsed_time / 1000.0f);
  win_printf_title(p_win, buff);

  // 온도
  if (p_kma->temperature.enable)
  {
    err = p_kma->temperature.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "기온", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->temperature.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "기온", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->temperature.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->temperature.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->temperature.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD, "기온",
                 data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 풍향
  if (p_kma->wind_direction_avg.enable)
  {
    err = p_kma->wind_direction_avg.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "풍향", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->wind_direction_avg.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f 도", AWS_WD, "풍향", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->wind_direction_avg.data);
        data_max = KMA_TO_GENERAL(p_kma->wind_direction_avg.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f 도,최대:%7.1f 도", AWS_WD, "풍향", data,
                 data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 풍속
  if (p_kma->wind_speed_avg.enable)
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "풍속", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->wind_speed_avg.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "풍속", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->wind_speed_avg.data);
        data_max = KMA_TO_GENERAL(p_kma->wind_speed_avg.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f m/s,최대:%7.1f m/s", AWS_WD, "풍속", data,
                 data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 순간 풍향
  if (p_kma->wind_direction_avg.enable &&
      (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR && page != eAWS_DATA_RAW))
  {
    err = p_kma->wind_direction_avg.err;
    if (err)
    {
      snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "순간 풍향");
    }
    else
    {
      snprintf(buff, sizeof(buff), "%-*s: %7.1f 도", AWS_WD, "순간 풍향",
               KMA_TO_GENERAL(p_kma->wind_direction_instant.data));
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 순간 풍속
  if (p_kma->wind_speed_avg.enable &&
      (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR && page != eAWS_DATA_RAW))
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "순간 풍속");
    }
    else
    {
      snprintf(buff, sizeof(buff), "%-*s: %7.1f m/s", AWS_WD, "순간 풍속",
               KMA_TO_GENERAL(p_kma->wind_speed_instant.data));
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 강수량
  if (p_kma->precipitation.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "강수량", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        uint32_t last_time = p_kma->precipitation.last_time;
        DATE_TIME_BUF nt;

        if (last_time == 0)
        {
          snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "강수량(time)");
        }
        else
        {
          time_cvt_secTotime(last_time, &nt);
          snprintf(buff, sizeof(buff), "%-*s: %04d-%02d-%02d %02d:%02d:%02d", AWS_WD,
                   "강수량(time)", nt.Year, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
        }
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.1f mm", AWS_WD, "강수량",
                 KMA_TO_GENERAL(p_kma->precipitation.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 기압
  if (p_kma->pressure.enable)
  {
    err = p_kma->pressure.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "기압", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->pressure.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f hPa", AWS_WD, "기압", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->pressure.data);
        data_min = KMA_TO_GENERAL(p_kma->pressure.min);
        data_max = KMA_TO_GENERAL(p_kma->pressure.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f hPa,최소:%7.1f hPa,최대:%7.1f hPa", AWS_WD,
                 "기압", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 강수유무
  if (p_kma->precipitation_presence.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation_presence.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "강수유무", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->precipitation_presence.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %5d", AWS_WD, "강수유무", (int)f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %5d", AWS_WD, "강수유무",
                 p_kma->precipitation_presence.data);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }
  // 적설
  if (p_kma->snowfall.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->snowfall.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "적설", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        int data = (int)p_kma->snowfall.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7d mm", AWS_WD, "적설", data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7d mm", AWS_WD, "적설", p_kma->snowfall.data);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 상대습도
  if (p_kma->relative_humidity.enable)
  {
    err = p_kma->relative_humidity.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "상대습도", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->relative_humidity.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "상대습도", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->relative_humidity.data);
        data_min = KMA_TO_GENERAL(p_kma->relative_humidity.min);
        data_max = KMA_TO_GENERAL(p_kma->relative_humidity.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f %%,최소:%7.1f %%,최대:%7.1f %%", AWS_WD,
                 "상대습도", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 강수량(0.1)
  if (p_kma->precipitation_fine.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation_fine.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "강수량(0.1)", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->precipitation_fine.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f mm", AWS_WD, "강수량(0.1)", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f mm", AWS_WD, "강수량(0.1)",
                 KMA_TO_GENERAL(p_kma->precipitation_fine.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 일사
  if (p_kma->solar_radiation.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->solar_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "일사", err_buf);
    }
    else
    {
      switch (page)
      {
        case eAWS_DATA_RAW:
        {
          float f_data = p_kma->solar_radiation.raw.f;
          snprintf(buff, sizeof(buff), "%-*s: %7.2f WJ/m2", AWS_WD, "일사", f_data);
          break;
        }
        case eAWS_DATA_AVG:
        {
          float solar_radiation = p_kma->solar_radiation.data;
          snprintf(buff, sizeof(buff), "%-*s: %7.1f WJ/m2", AWS_WD, "일사", solar_radiation);
          break;
        }
        default:
        {
          float solar_radiation = p_kma->solar_radiation.data;
          snprintf(buff, sizeof(buff), "%-*s: %7.1f KJ/m2", AWS_WD, "일사", solar_radiation);
          break;
        }
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 일조
  if (p_kma->sunshine_duration.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->sunshine_duration.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "일조", err_buf);
    }
    else
    {
      switch (page)
      {
        case eAWS_DATA_RAW:
        {
          float f_data = p_kma->sunshine_duration.raw.f;
          bool sunshine_duration = (f_data == 1.0f);
          snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "일조",
                   sunshine_duration ? "ON" : "OFF");
          break;
        }
        case eAWS_DATA_AVG:
        {
          bool sunshine_duration = (p_kma->sunshine_duration.data == 1);
          snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "일조",
                   sunshine_duration ? "ON" : "OFF");
          break;
        }
        default:
          snprintf(buff, sizeof(buff), "%-*s: %5d s", AWS_WD, "일조",
                   p_kma->sunshine_duration.data);
          break;
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }
  // 지면온도
  if (p_kma->surface_temperature.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->surface_temperature.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지면온도", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->surface_temperature.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지면온도", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD, "지면온도",
                 KMA_TO_TEMPERATURE(p_kma->surface_temperature.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 초상온도
  if (p_kma->grass_temperature.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->grass_temperature.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "초상온도", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->grass_temperature.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "초상온도", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD, "초상온도",
                 KMA_TO_TEMPERATURE(p_kma->grass_temperature.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 지중온도 5cm
  if (p_kma->soil_temperature_5cm.enable)
  {
    err = p_kma->soil_temperature_5cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지중온도 5cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_5cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지중온도 5cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD,
                 "지중온도 5cm", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 지중온도 10cm
  if (p_kma->soil_temperature_10cm.enable)
  {
    err = p_kma->soil_temperature_10cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지중온도 10cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_10cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지중온도 10cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD,
                 "지중온도 10cm", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 지중온도 20cm
  if (p_kma->soil_temperature_20cm.enable)
  {
    err = p_kma->soil_temperature_20cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지중온도 20cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_20cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지중온도 20cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD,
                 "지중온도 20cm", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 지중온도 30cm
  if (p_kma->soil_temperature_30cm.enable)
  {
    err = p_kma->soil_temperature_30cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지중온도 30cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_30cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지중온도 30cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD,
                 "지중온도 30cm", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }
  // 지중온도 50cm
  if (p_kma->soil_temperature_50cm.enable)
  {
    err = p_kma->soil_temperature_50cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지중온도 50cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_50cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지중온도 50cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD,
                 "지중온도 50cm", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 지중온도 1m
  if (p_kma->soil_temperature_1m.enable)
  {
    err = p_kma->soil_temperature_1m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지중온도 1m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_1m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지중온도 1m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD,
                 "지중온도 1m", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 지중온도 1.5m
  if (p_kma->soil_temperature_1_5m.enable)
  {
    err = p_kma->soil_temperature_1_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지중온도 1.5m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_1_5m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지중온도 1.5m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD,
                 "지중온도 1.5m", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 지중온도 3m
  if (p_kma->soil_temperature_3m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_temperature_3m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지중온도 3m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_3m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지중온도 3m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD,
                 "지중온도 3m", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 지중온도 5m
  if (p_kma->soil_temperature_5m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_temperature_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "지중온도 5m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_5m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "지중온도 5m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C,최소:%7.1f C,최대:%7.1f C", AWS_WD,
                 "지중온도 5m", data, data_min, data_max);
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 운고 1층
  if (p_kma->cloud_height_1st.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->cloud_height_1st.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "운고 1층", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->cloud_height_1st.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m", AWS_WD, "운고 1층", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.1f m", AWS_WD, "운고 1층",
                 KMA_TO_GENERAL(p_kma->cloud_height_1st.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 운고 2층
  if (p_kma->cloud_height_2nd.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->cloud_height_2nd.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "운고 2층", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->cloud_height_2nd.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m", AWS_WD, "운고 2층", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.1f m", AWS_WD, "운고 2층",
                 KMA_TO_GENERAL(p_kma->cloud_height_2nd.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 운고 3층
  if (p_kma->cloud_height_3rd.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->cloud_height_3rd.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "운고 3층", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->cloud_height_3rd.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m", AWS_WD, "운고 3층", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.1f m", AWS_WD, "운고 3층",
                 KMA_TO_GENERAL(p_kma->cloud_height_3rd.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 운량
  if (p_kma->cloud_amount.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->cloud_amount.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "운량", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->cloud_amount.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.1f", AWS_WD, "운량", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.1f", AWS_WD, "운량",
                 KMA_TO_GENERAL(p_kma->cloud_amount.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 시정
  if (p_kma->visibility.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->visibility.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "시정", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->visibility.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m", AWS_WD, "시정", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m", AWS_WD, "시정",
                 KMA_TO_GENERAL(p_kma->visibility.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 미세먼지 PM10
  if (p_kma->pm10_concentration.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->pm10_concentration.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "미세먼지 PM10", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->pm10_concentration.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f ug/m3", AWS_WD, "미세먼지 PM10", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f ug/m3", AWS_WD, "미세먼지 PM10",
                 KMA_TO_GENERAL(p_kma->pm10_concentration.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 미세먼지 PM2.5
  if (p_kma->pm25_concentration.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->pm25_concentration.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "미세먼지 PM2.5", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->pm25_concentration.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f ug/m3", AWS_WD, "미세먼지 PM2.5", f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f ug/m3", AWS_WD, "미세먼지 PM2.5",
                 KMA_TO_GENERAL(p_kma->pm25_concentration.data));
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }
  // 순복사
  if (p_kma->net_radiation.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->net_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "순복사", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->net_radiation.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f W/m2", AWS_WD, "순복사", f_data);
      }
      else if (page == eAWS_DATA_AVG || page == eAWS_DATA_1MIN)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f W/m2", AWS_WD, "순복사",
                 KMA_TO_RADI(p_kma->net_radiation.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: -", AWS_WD, "순복사");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 전천복사
  if (p_kma->total_radiation.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->total_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "전천복사", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->total_radiation.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f W/m2", AWS_WD, "전천복사", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f W/m2", AWS_WD, "전천복사",
                 KMA_TO_RADI(p_kma->total_radiation.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "전천복사");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 반사복사
  if (p_kma->reflected_radiation.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->reflected_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "반사복사", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->reflected_radiation.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f W/m2", AWS_WD, "반사복사", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f W/m2", AWS_WD, "반사복사",
                 KMA_TO_RADI(p_kma->reflected_radiation.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "반사복사");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 직달일사
  if (p_kma->direct_radiation.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->direct_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "직달일사", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->direct_radiation.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f W/m2", AWS_WD, "직달일사", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f W/m2", AWS_WD, "직달일사",
                 KMA_TO_RADI(p_kma->direct_radiation.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "직달일사");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 현재일기
  if (p_kma->current_weather.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->current_weather.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "현재일기", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->current_weather.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f", AWS_WD, "현재일기", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f", AWS_WD, "현재일기",
                 KMA_TO_GENERAL(p_kma->current_weather.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "현재일기");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }
  // 토양수분 10cm
  if (p_kma->soil_moisture_10cm.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_moisture_10cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "토양수분 10cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_moisture_10cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "토양수분 10cm", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "토양수분 10cm",
                 KMA_TO_GENERAL(p_kma->soil_moisture_10cm.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "토양수분 10cm");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 토양수분 20cm
  if (p_kma->soil_moisture_20cm.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_moisture_20cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "토양수분 20cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_moisture_20cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "토양수분 20cm", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "토양수분 20cm",
                 KMA_TO_GENERAL(p_kma->soil_moisture_20cm.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "토양수분 20cm");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 토양수분 30cm
  if (p_kma->soil_moisture_30cm.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_moisture_30cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "토양수분 30cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_moisture_30cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "토양수분 30cm", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "토양수분 30cm",
                 KMA_TO_GENERAL(p_kma->soil_moisture_30cm.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "토양수분 30cm");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 토양수분 50cm
  if (p_kma->soil_moisture_50cm.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_moisture_50cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "토양수분 50cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_moisture_50cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "토양수분 50cm", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "토양수분 50cm",
                 KMA_TO_GENERAL(p_kma->soil_moisture_50cm.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "토양수분 50cm");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 조도
  if (p_kma->illuminance.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->illuminance.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "조도", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->illuminance.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f klux", AWS_WD, "조도", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f klux", AWS_WD, "조도",
                 KMA_TO_ILLUMINANCE(p_kma->illuminance.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "조도");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }
  // 풍속 1.5m
  if (p_kma->wind_speed_1_5m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->wind_speed_1_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "풍속 1.5m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->wind_speed_1_5m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "풍속 1.5m", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "풍속 1.5m",
                 KMA_TO_GENERAL(p_kma->wind_speed_1_5m.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "풍속 1.5m");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 풍속 4m
  if (p_kma->wind_speed_4m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->wind_speed_4m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "풍속 4.0m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->wind_speed_4m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "풍속 4.0m", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "풍속 4.0m",
                 KMA_TO_GENERAL(p_kma->wind_speed_4m.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "풍속 4.0m");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 순간풍속 1.5m
  if (p_kma->instant_wind_speed_1_5m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->instant_wind_speed_1_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "순간풍속 1.5m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->instant_wind_speed_1_5m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "순간풍속 1.5m", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "순간풍속 1.5m",
                 KMA_TO_GENERAL(p_kma->instant_wind_speed_1_5m.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "순간풍속 1.5m");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 순간풍속 4.0m
  if (p_kma->instant_wind_speed_4m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->instant_wind_speed_4m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "순간풍속 4.0m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->instant_wind_speed_4m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "순간풍속 4.0m", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "순간풍속 4.0m",
                 KMA_TO_GENERAL(p_kma->instant_wind_speed_4m.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "순간풍속 4.0m");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }
  // 기온 0.5m
  if (p_kma->temperature_0_5m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->temperature_0_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "기온 0.5m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->temperature_0_5m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "기온 0.5m", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "기온 0.5m",
                 KMA_TO_TEMPERATURE(p_kma->temperature_0_5m.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "기온 0.5m");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 기온 4.0m
  if (p_kma->temperature_4m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->temperature_4m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "기온 4.0m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->temperature_4m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "기온 4.0m", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "기온 4.0m",
                 KMA_TO_TEMPERATURE(p_kma->temperature_4m.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "기온 4.0m");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 습도 0.5m
  if (p_kma->humidity_0_5m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->humidity_0_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "습도 0.5m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->humidity_0_5m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "습도 0.5m", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "습도 0.5m",
                 KMA_TO_GENERAL(p_kma->humidity_0_5m.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "습도 0.5m");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 습도 4.0m
  if (p_kma->humidity_4m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->humidity_4m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "습도 4.0m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->humidity_4m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "습도 4.0m", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "습도 4.0m",
                 KMA_TO_GENERAL(p_kma->humidity_4m.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "습도 4.0m");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  // 타코미터
  if (p_kma->tacometer.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->tacometer.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "타코미터", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->tacometer.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f rpm", AWS_WD, "타코미터", f_data);
      }
      else if (page == eAWS_DATA_AVG)
      {
        snprintf(buff, sizeof(buff), "%-*s: %7.2f rpm", AWS_WD, "타코미터",
                 KMA_TO_GENERAL(p_kma->tacometer.data));
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "타코미터");
      }
    }
    win_printf_row(p_win, row_count++, buff);
  }

  p_win->total_items[page] = ALIGN_UP(row_count, p_win->view_row);



  // Fill remaining rows with blank lines to maintain consistent window size
  while (p_win->current_row < p_win->view_row)
  {
    win_printf_row(p_win, row_count++, "");
  }

  win_print_close(p_win);

}

#define CHARGER_WD 15
void draw_config(win_t *p_win)
{
  int row_count=0;

  int win_height = p_win->view_row + 3;

  p_win->total_pages = 1;
  p_win->current_row = 0;
  calculate_window_position(p_win, p_win->view_col, win_height);


  win_printf_title(p_win, "설정");



  while (p_win->current_row < p_win->view_row)
  {
    win_printf_row(p_win, row_count++, "");
  }

  win_print_close(p_win);
}
int32_t aws_menu_veiw(void)
{
  win_t system_win;
  win_t rain_win;
  win_t cdma_win;
  win_t direct_win;
  win_t eth_win;
  win_t aws_win;
  win_t charger_win;
  win_t config_win;
  win_t *windows[7];
  int window_count = 0;
  int current_win = 5;//aws
  app_mode_t mode = MODE_SELECT;


  io_printf(VT100_CLEAR_SCREEN);
  io_printf(VT100_CURSOR_OFF);


  create_win(&system_win, 0, 0, 6, 26);
  create_win(&rain_win, 0, 0, 6, 26);
  create_win(&cdma_win, 0, 0, 6, 31);
  create_win(&direct_win, 0, 0, 6, 31);
  create_win(&eth_win, 0, 0, 15, 31);
  create_win(&aws_win, 0, 0, 19, 62);
  create_win(&charger_win, 0, 0, 6, 26);
  create_win(&config_win, 0, 0, 8, 22);

  while (1)
  {
    io_printf(VT100_CURSOR_HOME);
    reset_layout();
    window_count = 0;

    reset_win(&system_win);
    reset_win(&rain_win);
    reset_win(&cdma_win);
    reset_win(&direct_win);
    reset_win(&eth_win);
    reset_win(&aws_win);
    reset_win(&charger_win);
    reset_win(&config_win);

    windows[window_count++] = &system_win;
    windows[window_count++] = &rain_win;
    windows[window_count++] = &charger_win;
    if (get_config_app()->cdma_use)
      windows[window_count++] = &cdma_win;
    if (get_config_app()->direct_use)
      windows[window_count++] = &direct_win;
    windows[window_count++] = &aws_win;
    if (get_config_app()->eth_use)
      windows[window_count++] = &eth_win;
    windows[window_count++] = &config_win;



    if (current_win < window_count)
    {
      if (mode == MODE_NAVIGATE)
      {
        windows[current_win]->is_focused = 1;
      }
      else
      {
        windows[current_win]->is_selected = 1;
      }
    }

    draw_system(&system_win);
    draw_rain(&rain_win);
    draw_charger(&charger_win);
    if (get_config_app()->cdma_use)
      draw_cdma(&cdma_win);
    if (get_config_app()->direct_use)
      draw_direct(&direct_win);
    draw_aws(&aws_win);
    if (get_config_app()->eth_use)
      draw_eth(&eth_win);
    draw_config(&config_win);

    if (window_count == 0)
    {
      io_printf("No windows to display\n");
      break;
    }

    if (current_win >= window_count)
    {
      current_win = 0;
    }

    layout_t *layout = get_layout();
    io_printf("\x1B[%d;1H", layout->next_y + layout->row_height + 2);
    io_printf("");

    int key = view_get_key_input(1000);

    if (key == KEY_ENTER)
    {  // Enter
      if (mode == MODE_NAVIGATE)
      {
        mode = MODE_SELECT;
      }
      else if (mode == MODE_SELECT)
      {
        mode = MODE_NAVIGATE;
      }
    }
    else if (key == KEY_BREAK)
    {
      break;
    }
    else if (mode == MODE_NAVIGATE)
    {
      handle_navigation_ptr(windows, window_count, &current_win, key);
    }
    else if (mode == MODE_SELECT)
    {
      handle_scroll(windows[current_win], key);
    }
    
  }

  io_printf("\r\n");
  return 1;
}