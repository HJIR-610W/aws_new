
#include "task_menu.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>


#include "app_key.h"
#include "app_charger.h"
#include "app_screen.h"
#include "aws_data.h"
#include "bsp.h"
#include "bsp_di.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "const_string.h"
#include "console_utile.h"
#include "task_cellular.h"
#include "task_client.h"
#include "task_direct.h"
#include "task_logging.h"
#include "task_measure.h"
#include "task_tcpServer.h"
#include "task_system.h"
#include "tcp_define.h"
#include "time_define.h"
#include "util_time.h"
#include "view_driver.h"
#include "pcb_define.h"
#include "os_user_def.h"
#include "menu\menu_setup.h"
#include "util_stdio.h"
#include "drv_system.h"
#include "FreeRTOS.h"
#include "schedule.h"


extern exec_time_t g_exec_250ms_time;  // Task 실행 시간 측정용
extern exec_time_t g_exec_1s_time;            // Task 실행 시간 측정용
extern void make_error_string(uint8_t error, char *buffer, uint32_t buffer_size);
extern const char *linkStatusList[3];
extern const char *generalStatusList[2];

#define SCREEN_COLS 21
#define SCREEN_ROWS 8
#define SCREEN_OFF_TIMEOUT_MS 600000

const osThreadAttr_t kMenuTask_attributes = {
    .name = "menu",
    .stack_size = TASK_STACK(TASK_MENU_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_MENU_DEF),
};


#define SYSTEM_WD 8
extern uint8_t BSP_PlatformIsDetected(void);;

void draw_system_page(screen_page_t* p_win)
{
  char buff[SCREEN_COLS + 1];
  const char *message;

  screen_page_start(p_win);
  make_centered(buff, sizeof(buff), "SYSTEM", SCREEN_COLS);
  screen_page_printf(p_win,"%s",buff);

  screen_page_printf(p_win, "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year,
                 Date_Time.Month, Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
  screen_page_printf(p_win, "%-*s:%d", SYSTEM_WD, "ID", get_config_app()->id);
  screen_page_printf(p_win, "%-*s:%s", SYSTEM_WD, "DOOR",
                    ITEM_LIST(is_door_opened(), doorStatusList_lcd));

  message = get_logging_system()->status_group?"ERROR":"NORMAL";
  screen_page_printf(p_win, "%-*s:%s", SYSTEM_WD,   "LOGGING", message);
  screen_page_printf(p_win, "%-*s:%.1f V", SYSTEM_WD, "SYS VOLT", drv_system_read(DRV_SYS_BATTERY));
  screen_page_printf(p_win, "%-*s:%.1f C", SYSTEM_WD, "SYS TEMP", drv_system_read(DRV_SYS_TEMPERATURE));
  screen_page_printf(p_win, "%-*s:%s", SYSTEM_WD, "SD CARD", ITEM_LIST(BSP_PlatformIsDetected(), sdcard_status_list_lcd));

  if (get_config_app()->ac_active)
  {
    screen_page_printf(p_win, "%-*s:%s", SYSTEM_WD, "AC", "ON");
  }

  screen_page_clear(p_win);
}

#define RAIN_WD 10
void draw_rain_page(screen_page_t *p_win)
{
  char buff[SCREEN_COLS + 1];

  screen_page_start(p_win);

  make_centered(buff, sizeof(buff), "RAIN", SCREEN_COLS);
  screen_page_printf(p_win, "%s",buff);

  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "YESTERDAY",
                     get_rainfall()->rainfall_yesterday);

  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "TODAY",
                     get_rainfall()->rainfall_today);

  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "1MIN",
                     get_rainfall()->rainfall_1min);

  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "10MIN",
                     get_rainfall()->rainfall_10min);

  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "HOUR",
                     get_rainfall()->rainfall_hourly);

  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "YEAR",
                     get_rainfall()->rainfall_yearly);

  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "MONTH",
                     get_rainfall()->rainfall_monthly);

  screen_page_clear(p_win);

}

#define CHARGER_WD 10
void draw_charger_page(screen_page_t *p_win)
{
  char buff[SCREEN_COLS + 1];
  char temp[20];
  uint8_t err;

  screen_page_start(p_win);
  make_centered(buff, sizeof(buff), "CHARGER", SCREEN_COLS);
  screen_page_printf(p_win, "%s",buff);
  read_chargerStatus(temp, sizeof(temp));
  screen_page_printf(p_win, "%-*s:%s", CHARGER_WD, "STATUS", temp);

  if (is_chargerValid())
  {
    screen_page_printf(p_win, "%-*s:%.2f", CHARGER_WD, "SOLAR V",
                   read_solarVoltage1(&err));
    screen_page_printf(p_win, "%-*s:%.2f", CHARGER_WD, "SOLAR A",
                   read_solarCurrent1(&err));
    screen_page_printf(p_win, "%-*s:%.2f", CHARGER_WD, "BATTERY V",
                   read_batteryVoltage1(&err));
    screen_page_printf(p_win, "%-*s:%.2f", CHARGER_WD, "LOAD A", read_loadCurrent1(&err));
  }
  else
  {
    screen_page_printf(p_win, "%-*s:%s", CHARGER_WD, "SOLAR V", "-");
    screen_page_printf(p_win, "%-*s:%s", CHARGER_WD, "SOLAR A", "-");
    screen_page_printf(p_win, "%-*s:%s", CHARGER_WD, "BATTERY V", "-");
    screen_page_printf(p_win, "%-*s:%s", CHARGER_WD, "LOAD A", "-");

  }

  screen_page_clear(p_win);

}

#define CDMA_WD 10
void draw_cdma_page(screen_page_t *p_win)
{

  char buff[SCREEN_COLS + 1];
  char num[20];
  uint32_t last_time;
  DATE_TIME_BUF nt;

  screen_page_start(p_win);
  make_centered(buff, sizeof(buff), "CDMA", SCREEN_COLS);
  screen_page_printf(p_win, buff);
  screen_page_printf(p_win, "%-*s:%s", CDMA_WD, "LINK",
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
  screen_page_printf(p_win, "%-*s:%s", CDMA_WD, "PHONE", num);

  if (get_cdma_system()->rssi == -1)
  {
    screen_page_printf(p_win, "%-*s:-", CDMA_WD, "RSSI");
  }
  else
  {
    screen_page_printf(p_win, "%-*s:%d", CDMA_WD, "RSSI", get_cdma_system()->rssi);
  }

  screen_page_printf(p_win, "%-*s:%d", CDMA_WD, "RX", get_cdma_system()->rx_cnt);
  screen_page_printf(p_win, "%-*s:%d", CDMA_WD, "TX", get_cdma_system()->tx_cnt);

  last_time = get_cdma_system()->last_recv_time;
  if (last_time == 0)
  {
    screen_page_printf(p_win, "RT:-");
  }
  else
  {
    time_cvt_secTotime(last_time, &nt);
    screen_page_printf(p_win, "RT:%02d-%02d-%02d %02d:%02d:%02d", 
                   nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }


  last_time = get_cdma_system()->last_send_time;
  if (last_time == 0)
  {
    screen_page_printf(p_win, "TT:-");
  }
  else
  {
    time_cvt_secTotime(last_time, &nt);
    screen_page_printf(p_win, "TT:%02d-%02d-%02d %02d:%02d:%02d",
                   nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  screen_page_clear(p_win);
}

#define DIRECT_WD 8
void draw_direct_page(screen_page_t *p_win)
{

  char buff[SCREEN_COLS + 1];
  DATE_TIME_BUF nt;
  uint32_t last_time;
  uint32_t remain_sec;

  make_centered(buff, sizeof(buff), "DIRECT", SCREEN_COLS);

  screen_page_start(p_win);
  screen_page_printf(p_win, "%s",buff);
  screen_page_printf(p_win, "%-*s:%s", DIRECT_WD, "LINK",
                 ITEM_LIST(get_direct_system()->link_status, linkStatusList));

  remain_sec = (uint32_t)(get_direct_system()->linkdown_remain_ms / 1000.0);
  screen_page_printf(p_win, "%-*s:%d", DIRECT_WD, "TIMEOUT", remain_sec);
  screen_page_printf(p_win, "%-*s:%d", DIRECT_WD, "RX", get_direct_system()->rx_cnt);
  screen_page_printf(p_win, "%-*s:%d", DIRECT_WD, "TX", get_direct_system()->tx_cnt);

  last_time = get_direct_system()->last_recv_time;
  if (last_time == 0)
  {
    screen_page_printf(p_win, "RT:-");
  }
  else
  {
    time_cvt_secTotime(last_time, &nt);
    screen_page_printf(p_win, "RT:%02d-%02d-%02d %02d:%02d:%02d",
                   nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  last_time = get_direct_system()->last_send_time;
  if (last_time == 0)
  {
    screen_page_printf(p_win, "TT:-");
  }
  else
  {
    time_cvt_secTotime(last_time, &nt);
    screen_page_printf(p_win, "TT:%02d-%02d-%02d %02d:%02d:%02d",
                   nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
  }

  screen_page_clear(p_win);
}

//:192.168.123.123
#define ETH_WD 2
void draw_ethernet_page(screen_page_t *p_win)
{
  char buff[SCREEN_COLS + 1];
  uint8_t tx_cnt[ETH_CLIENT_MAX];
  uint8_t rx_cnt[ETH_CLIENT_MAX];
  uint32_t last_time;
  DATE_TIME_BUF nt;
  eLINK_STATUS_t link_status[ETH_CLIENT_MAX];

  make_centered(buff, sizeof(buff), "ETHERNET", SCREEN_COLS);

  screen_page_start(p_win);
  screen_page_printf(p_win, "%s",buff);

  if (get_config_app()->eth_mode == eETH_MODE_CLINET)
  {
    screen_page_printf(p_win, "%-*s:%s", ETH_WD, "LINK",
                   ITEM_LIST(get_tcp_client_system()->link_status, linkStatusList_lcd));

    screen_page_printf(p_win, "%-*s:%d", ETH_WD, "RX", get_tcp_client_system()->rx_cnt);
    screen_page_printf(p_win, "%-*s:%d", ETH_WD, "TX", get_tcp_client_system()->tx_cnt);



    last_time = get_tcp_client_system()->last_recv_time;
    if (last_time == 0)
    {
      screen_page_printf(p_win, "RT:-");
    }
    else
    {
      time_cvt_secTotime(last_time, &nt);
      //"RT:25-07-22 10:10:2 "
      screen_page_printf(p_win, "RT:%02d-%02d-%02d %02d:%02d:%02d",
                     nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
    }

    last_time = get_tcp_client_system()->last_send_time;
    if (last_time == 0)
    {
      screen_page_printf(p_win, "TT:-");
    }
    else
    {
      time_cvt_secTotime(last_time, &nt);
      screen_page_printf(p_win, "TT:%02d-%02d-%02d %02d:%02d:%02d", 
                     nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
    }
  }
  else
  {
    for (int i = 0; i < ETH_CLIENT_MAX; i++)
    {
      link_status[i] = get_tcp_system(i)->link_status;
      tx_cnt[i] = get_tcp_system(i)->tx_cnt;
      rx_cnt[i] = get_tcp_system(i)->rx_cnt;
    }

    for (int i = 0; i < ETH_CLIENT_MAX; i++)
    {
      //L0:D/192.168.123.123 
      screen_page_printf(p_win, "L%d:%s(%s)", i,
                     ITEM_LIST(link_status[i], ethlinkStatusList_lcd),
                     get_tcp_system(i)->client_ip_str);

      screen_page_printf(p_win, "%-*s:%d", ETH_WD, "RX", rx_cnt[i]);
      screen_page_printf(p_win, "%-*s:%d", ETH_WD, "TX", tx_cnt[i]);



      last_time = get_tcp_system(i)->last_recv_time;
      if (last_time == 0)
      {
        screen_page_printf(p_win, "RT:-");
      }
      else
      {
        time_cvt_secTotime(last_time, &nt);
        screen_page_printf(p_win, "RT:%02d-%02d-%02d %02d:%02d:%02d",
                       nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
      }

      last_time = get_tcp_system(i)->last_send_time;
      if (last_time == 0)
      {
        screen_page_printf(p_win, "TT:-");
      }
      else
      {
        time_cvt_secTotime(last_time, &nt);
        screen_page_printf(p_win, "TT:%02d-%02d-%02d %02d:%02d:%02d",
                       nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
      }
    }
  }

  screen_page_clear(p_win); 
}

#define AWS_WD 7
void draw_aws_page(screen_page_t *p_win, eAWS_DATA_MIN_t min)
{
  const char *aws_title_list[] = {"AVG", "1MIN", "10MIN", "HOUR", "RAW"};
  char err_buf[32];
  uint8_t err;
  float data, data_min, data_max;
  kma_data_ex_t *p_kma = NULL;

  screen_page_start(p_win);
  screen_page_printf(p_win, "AWS %s %.2fs/%.2fs", aws_title_list[(int)min],
                 (float)g_exec_250ms_time.elapsed_time / 1000.0f,
                 (float)g_exec_1s_time.elapsed_time / 1000.0f);

  p_kma = get_kma_data((eAWS_DATA_MIN_t)min);

  if (p_kma->temperature.enable)
  {
    err = p_kma->temperature.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "TEMP", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->temperature.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "TEMP", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->temperature.data);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "TEMP", data);
      }
    }
  }

  // 풍향
  if (p_kma->wind_direction_avg.enable)
  {
    err = p_kma->wind_direction_avg.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "WIND D", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->wind_direction_avg.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f deg", AWS_WD, "WIND D", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->wind_direction_avg.data);
        data_max = KMA_TO_GENERAL(p_kma->wind_direction_avg.max);
        screen_page_printf(p_win, "%-*s:%6.1f deg", AWS_WD, "WIND D", data);
      }
    }
  }

  // 풍속
  if (p_kma->wind_speed_avg.enable)
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "WIND S", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->wind_speed_avg.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f m/s", AWS_WD, "WIND S", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->wind_speed_avg.data);
        data_max = KMA_TO_GENERAL(p_kma->wind_speed_avg.max);
        screen_page_printf(p_win, "%-*s:%6.1f m/s", AWS_WD, "WIND S", data);
      }
    }
  }

  // 순간 풍향
  if (p_kma->wind_direction_avg.enable &&
      (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR && min != eAWS_DATA_RAW))
  {
    err = p_kma->wind_direction_avg.err;
    if (err)
    {
      screen_page_printf(p_win, "%-*s:--", AWS_WD, "WIND DG");
    }
    else
    {
      screen_page_printf(p_win, "%-*s:%6.1f deg", AWS_WD, "WIND DG",
                        KMA_TO_GENERAL(p_kma->wind_direction_instant.data));
    }
  }

  // 순간 풍속
  if (p_kma->wind_speed_avg.enable &&
      (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR && min != eAWS_DATA_RAW))
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      screen_page_printf(p_win, "%-*s:--", AWS_WD, "WIND SG");
    }
    else
    {
      screen_page_printf(p_win, "%-*s:%6.1f m/s", AWS_WD, "WIND SG",
                     KMA_TO_GENERAL(p_kma->wind_speed_instant.data));
    }
  }

  // 강수량
  if (p_kma->precipitation.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "RAIN", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        uint32_t last_time = p_kma->precipitation.last_time;
        DATE_TIME_BUF nt;

        if (last_time == 0)
        {
          screen_page_printf(p_win, "%-*s:--", AWS_WD, "RAIN(t)");
        }
        else
        {
          time_cvt_secTotime(last_time, &nt);
          //RAIN(t):250101000000
          screen_page_printf(p_win, "RAIN(t):%02d%02d%02d%02d%02d%02d", 
                            nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
        }
      }
      else
      {
        screen_page_printf(p_win, "%-*s:%6.1f mm", AWS_WD, "RAIN",
                       KMA_TO_GENERAL(p_kma->precipitation.data));
      }
    }
  }
  // 기압 //BAROMETER:1000.0hpa
  if (p_kma->pressure.enable)
  {
    err = p_kma->pressure.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "BARO", err_buf);
    }
    else 
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->pressure.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f hpa", AWS_WD, "BARO", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->pressure.data);
        data_min = KMA_TO_GENERAL(p_kma->pressure.min);
        data_max = KMA_TO_GENERAL(p_kma->pressure.max);
        screen_page_printf(p_win, "%-*s:%6.1f hpa", AWS_WD, "BARO", data);
      }
    }
  }

  // 강수유무
  if (p_kma->precipitation_presence.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation_presence.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "RAIN_P", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW )
      {

        bool rain_p = p_kma->precipitation_presence.raw.b ;
        screen_page_printf(p_win, "%-*s: %s", AWS_WD, "RAIN_P", rain_p?"ON":"OFF");
      }
      else if ( min == eAWS_DATA_AVG)
      {
        uint16_t data = p_kma->precipitation_presence.data;
        bool rain_p = (data == 10) ? true : false;
        screen_page_printf(p_win, "%-*s: %s", AWS_WD, "RAIN_P", rain_p ? "ON" : "OFF");
      }
      else
      {
        screen_page_printf(p_win, "%-*s: %4d", AWS_WD, "RAIN_P",
                 p_kma->precipitation_presence.data);
      }
    }
  }
  // 적설
  if (p_kma->snowfall.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    err = p_kma->snowfall.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "SNOW", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        int data = (int)p_kma->snowfall.raw.f;
        screen_page_printf(p_win, "%-*s:%5.1f cm", AWS_WD, "SNOW", (float)data/10.0f);
      }
      else
      {
        screen_page_printf(p_win, "%-*s:%5.1f cm", AWS_WD, "SNOW", (float)p_kma->snowfall.data/10.0);
      }
    }
  }

  // 상대습도
  if (p_kma->relative_humidity.enable)
  {
    err = p_kma->relative_humidity.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "HUMI", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->relative_humidity.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f %%", AWS_WD, "HUMI", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->relative_humidity.data);
        data_min = KMA_TO_GENERAL(p_kma->relative_humidity.min);
        data_max = KMA_TO_GENERAL(p_kma->relative_humidity.max);
        screen_page_printf(p_win, "%-*s:%6.1f %%", AWS_WD,"HUMI", data);
      }
    }
  }
#define SOLAR_R_WD 7
  // 일사 "SOLAR R: 123.1 kW/m2"
  if (p_kma->solar_radiation.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    err = p_kma->solar_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "SOLAR R", err_buf);
    }
    else
    {
      switch (min)
      {
        case eAWS_DATA_RAW:
        {
          float f_data = p_kma->solar_radiation.raw.f;
          screen_page_printf(p_win, "%-*s:%6.1f W/m2", SOLAR_R_WD, "SOLAR R", f_data);
          break;
        }
        case eAWS_DATA_AVG:
        {
          float solar_radiation = (float)get_sunshine_r()->sunshine_r_1min_acc/1000.0f;
          screen_page_printf(p_win, "%-*s:%6.1f kJ/m2", SOLAR_R_WD, "SOLAR R", solar_radiation);
          break;
        }
        break;
        default:
        {
          float solar_radiation = p_kma->solar_radiation.data*10;
          screen_page_printf(p_win, "%-*s:%6.1f kJ/m2", SOLAR_R_WD, "SOLAR R", solar_radiation);
          break;
        }

      }
    }
  }


  if (p_kma->sunshine_duration.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
     uint32_t solar_d_today = get_sunshine()->sunshine_today;
       err = p_kma->sunshine_duration.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "SOLAR D", err_buf);
    }
    else
    {
      switch (min)
      {
        case eAWS_DATA_RAW:
        {
          float f_data = p_kma->sunshine_duration.raw.f;
          screen_page_printf(p_win, "%-*s:   %s", AWS_WD, "SOLAR D", (f_data == 1.0f) ? "ON" : "OFF");
          break;
        }
        case eAWS_DATA_AVG:
          screen_page_printf(p_win, "%-*s:%6d sec", AWS_WD, "SOLAR D", solar_d_today);
        break;
        case eAWS_DATA_1MIN:
        {
          solar_d_today = p_kma->sunshine_duration.data;
          screen_page_printf(p_win, "%-*s:%6d sec", AWS_WD, "SOLAR D", solar_d_today);
        }
        break;
      }
    }
  }

  // 지중온도 5cm
  if (p_kma->soil_temperature_5cm.enable)
  {
    err = p_kma->soil_temperature_5cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "ST 5cm", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_5cm.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 5cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.max);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD,
                          "ST 5cm", data);
      }
    }
  }

  // 지중온도 10cm
  if (p_kma->soil_temperature_10cm.enable)
  {
    err = p_kma->soil_temperature_10cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "ST 10cm", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_10cm.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 10cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.max);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 10cm", data);
      }
    }
  }

  // 지중온도 20cm
  if (p_kma->soil_temperature_20cm.enable)
  {
    err = p_kma->soil_temperature_20cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "ST 20cm", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_20cm.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 20cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.max);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 20cm", data);
      }
    }
  }

  // 지중온도 30cm
  if (p_kma->soil_temperature_30cm.enable)
  {
    err = p_kma->soil_temperature_30cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "ST 30cm", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_30cm.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 30cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.max);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 30cm", data);
      }
    }
  }
  // 지중온도 50cm
  if (p_kma->soil_temperature_50cm.enable)
  {
    err = p_kma->soil_temperature_50cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "ST 50cm", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_50cm.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 50cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.max);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD,
                          "ST 50cm", data);
      }
    }
  }

  // 지중온도 1m
  if (p_kma->soil_temperature_1m.enable)
  {
    err = p_kma->soil_temperature_1m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "ST 1m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_1m.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 1m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.max);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 1m", data);
      }
    }
  }

  // 지중온도 1.5m
  if (p_kma->soil_temperature_1_5m.enable)
  {
    err = p_kma->soil_temperature_1_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "ST 1.5m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_1_5m.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 1.5m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.max);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 1.5m", data);
      }
    }
  }

  // 지중온도 3m
  if (p_kma->soil_temperature_3m.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_temperature_3m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "ST 3m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_3m.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 3m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.max);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 3m", data);
      }
    }
  }

  // 지중온도 5m
  if (p_kma->soil_temperature_5m.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_temperature_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "ST 5m", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_5m.raw.f;
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 5m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.max);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "ST 5m", data);
      }
    }
  }
screen_page_clear(p_win);



}








void config_set_menu(void)
{

}

#define PAGE_SYSTEM  0
#define PAGE_RAIN    1
#define PAGE_CHARGER 2
#define PAGE_CDMA    3
#define PAGE_DIRECT  4
#define PAGE_ETH       5
#define PAGE_AWS_AVG   6
#define PAGE_AWS_1MIN  7
#define PAGE_AWS_10MIN 8
#define PAGE_AWS_HOUR  9
#define PAGE_AWS_RAW   10
#define PAGE_MAX       11


static const uint8_t s_hwajin_logo[64][16] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x03, 0x03, 0x03, 0xF0, 0x3F, 0xFF, 0x3F, 0xFF, 0x30, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x03, 0x03, 0x03, 0xF0, 0x3F, 0xFF, 0x3F, 0xFF, 0x30, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x03, 0x03, 0x0C, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x3C, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x03, 0x03, 0x0C, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x3C, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x03, 0x03, 0x30, 0x03, 0x00, 0x30, 0x00, 0xC0, 0x3F, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x03, 0x03, 0x30, 0x03, 0x00, 0x30, 0x00, 0xC0, 0x3F, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x3F, 0xF3, 0x03, 0x03, 0x30, 0x03, 0x00, 0x30, 0x00, 0xC0, 0x33, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x3F, 0xF3, 0x03, 0x03, 0x30, 0x03, 0x00, 0x30, 0x00, 0xC0, 0x33, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x33, 0x33, 0x3F, 0xFF, 0x00, 0x30, 0x00, 0xC0, 0x30, 0xCC, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x33, 0x33, 0x3F, 0xFF, 0x00, 0x30, 0x00, 0xC0, 0x30, 0xCC, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x33, 0x33, 0x30, 0x03, 0x30, 0x30, 0x00, 0xC0, 0x30, 0x3C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x33, 0x33, 0x30, 0x03, 0x30, 0x30, 0x00, 0xC0, 0x30, 0x3C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0xCC, 0xCF, 0x30, 0x03, 0x30, 0x30, 0x00, 0xC0, 0x30, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0xCC, 0xCF, 0x30, 0x03, 0x30, 0x30, 0x00, 0xC0, 0x30, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x03, 0x03, 0x30, 0x03, 0x0F, 0xC0, 0x3F, 0xFF, 0x30, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x33, 0x03, 0x03, 0x30, 0x03, 0x0F, 0xC0, 0x3F, 0xFF, 0x30, 0x0C, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};


void print_logo(void)
{
  uint8_t data;
  for (int row = 0; row < 64; row++)
  {
    for (int col = 0; col < 16; col++)
    {
      data = s_hwajin_logo[row][col];

      for (int b = 0; b < 8; b++)
      {
        if (data & (0x80 >> b))
        {
          screen_set_pixel(col * 8 + b, row, 1);
        }
        else
        {
          screen_set_pixel(col * 8 + b, row, 0);
        }
      }
    }
  }
  screen_refresh();
  osDelay(1000);
}

void menuTask(void *arg)
{
  int32_t key;
  int32_t page_count = 0;
  int32_t page_list[PAGE_MAX];
  uint32_t screen_off_time;
  screen_page_t lcd_win;
  
  screen_init();
  
  print_logo();

  screen_page_create(&lcd_win);

  lcd_win.chunk_scroll_use = 1;// view_row 단위로 스크롤
  lcd_win.multi_page_use = 1;  //하나의 창에 여러개의 페이지 구성 LEFT,RIGHT 키 사용

  screen_off_time = OS_GET_TICK();
  while (1)
  {
    page_count = 0;
    page_list[page_count++] = PAGE_SYSTEM;
    page_list[page_count++] = PAGE_RAIN;
    page_list[page_count++] = PAGE_CHARGER;
    
    if (get_config_app()->cdma_active)
      page_list[page_count++] = PAGE_CDMA;
    if (get_config_app()->direct_active)
      page_list[page_count++] = PAGE_DIRECT;
    if (get_config_app()->eth_active)
      page_list[page_count++] = PAGE_ETH;
    
    page_list[page_count++] = PAGE_AWS_AVG;
    page_list[page_count++] = PAGE_AWS_1MIN;
    // page_list[page_count++] = PAGE_AWS_10MIN; 구형에서는 표시 했지만 현재 불필요
    // page_list[page_count++] = PAGE_AWS_HOUR; 구형에서는 표시 했지만 현재 불필요
    page_list[page_count++] = PAGE_AWS_RAW;
    
    lcd_win.total_pages = page_count;
                   
  switch (page_list[lcd_win.current_page])
  {
    case PAGE_SYSTEM:
    draw_system_page(&lcd_win);
    break;
    case PAGE_RAIN:
    draw_rain_page(&lcd_win);
    break;
    case PAGE_CHARGER:
    draw_charger_page(&lcd_win);
    break;
    case PAGE_CDMA:
    draw_cdma_page(&lcd_win);
    break;
    case PAGE_DIRECT:
    draw_direct_page(&lcd_win);
    break;
    case PAGE_ETH:
    draw_ethernet_page(&lcd_win);
    break;
    case PAGE_AWS_AVG:
    draw_aws_page(&lcd_win, eAWS_DATA_AVG);
    break;
    case PAGE_AWS_1MIN:
    draw_aws_page(&lcd_win, eAWS_DATA_1MIN);
    break;
    case PAGE_AWS_10MIN:
    draw_aws_page(&lcd_win, eAWS_DATA_10MIN);
    break;
    case PAGE_AWS_HOUR:
    draw_aws_page(&lcd_win, eAWS_DATA_HOUR);
    break;
    case PAGE_AWS_RAW:
    draw_aws_page(&lcd_win, eAWS_DATA_RAW);
    break;
    default:
    break;
    }
           
    screen_refresh();

    key =  get_button_key(1000);

    if (key == KEY_CODE_CTRL_A)
    {
      setup_root();
      screen_off_time = OS_GET_TICK();
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_page_handle(&lcd_win, key);
      screen_off_time = OS_GET_TICK();
    }

    if ((OS_GET_TICK() - screen_off_time) > SCREEN_OFF_TIMEOUT_MS)
    {
      screen_off();
      key = get_button_key(0xFFFFFFFF);//무한 대기 
      screen_off_time = OS_GET_TICK();
    }

    }
}

void menuTask_init(void)
{
  osThreadNew(menuTask, NULL, &kMenuTask_attributes);
}



