
#include "task_menu.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "app_charger.h"
#include "app_key.h"
#include "app_screen.h"
#include "app_version.h"
#include "aws_data.h"
#include "bsp.h"
#include "bsp_di.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "console_utile.h"
#include "const_string.h"
#include "drv_system.h"
#include "FreeRTOS.h"
#include "menu\menu_setup.h"
#include "menu_handler.h"
#include "os_user_def.h"
#include "pcb_define.h"
#include "schedule.h"
#include "system_err.h"
#include "task_cellular.h"
#include "task_client.h"
#include "task_direct.h"
#include "task_logging.h"
#include "task_measure.h"
#include "task_menu_define.h"
#include "task_system.h"
#include "task_tcpServer.h"
#include "tcp_define.h"
#include "test_menu_setup.h"
#include "util_time.h"
#include "util_stdio.h"
#include "util_time.h"
#include "view_driver.h"

extern exec_time_t g_exec_250ms_time;  // Task 실행 시간 측정용
extern exec_time_t g_exec_1s_time;            // Task 실행 시간 측정용
extern const char *link_status_list_eng[3];
extern const char *generalStatusList[2];
extern void make_error_string(uint8_t error, char *buffer, uint32_t buffer_size);
extern uint8_t BSP_PlatformIsDetected(void);
extern int32_t g_rain_off_remain_time;
extern bool g_rain_timer_counting_down;


#define SCREEN_COLS 21
#define SCREEN_ROWS 8
#define SCREEN_OFF_TIMEOUT_MS 10000

const osThreadAttr_t kMenuTask_attributes = {
    .name = "menu",
    .stack_size = TASK_STACK(TASK_MENU_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_MENU_DEF),
};

const osThreadAttr_t kBootProgressTask_attributes = {
    .name = "progress",
    .stack_size = TASK_STACK(TASK_BOOT_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_BOOT_DEF),
};

const osThreadAttr_t kMenuTestTask_attributes = {
    .name = "test",
    .stack_size = TASK_STACK(TASK_MENU_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_MENU_DEF),
};

osThreadId_t g_menu_task_id;
#define SYSTEM_WD 8

void draw_system_page(screen_page_t *p_win)
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
                    ITEM_LIST(is_door_opened(), door_status_list_eng));

  message = get_logging_system()->status_group?"ERROR":"NORMAL";
  screen_page_printf(p_win, "%-*s:%s", SYSTEM_WD,   "LOGGING", message);
  screen_page_printf(p_win, "%-*s:%.1fV", SYSTEM_WD, "SYS VOLT", drv_system_read(DRV_SYS_BATTERY));
  screen_page_printf(p_win, "%-*s:%.1fC", SYSTEM_WD, "SYS TEMP", drv_system_read(DRV_SYS_TEMPERATURE));
  screen_page_printf(p_win, "%-*s:%s", SYSTEM_WD, "SD CARD", ITEM_LIST(BSP_PlatformIsDetected(), sdcard_status_list_lcd));


  {
    uint8_t major;
    uint8_t minor;
    uint8_t fix;
    uint8_t rel;
    DATE_TIME_BUF build_time;

    get_app_version(&major, &minor, &fix, &rel);
    screen_page_printf(p_win, "%-*s:%d.%d.%d.%d", SYSTEM_WD, "VER", major,minor,fix,rel);
    get_app_build(&build_time);
    screen_page_printf(p_win, "BUILD:%04d%02d%02d%02d%02d%02d", build_time.Year,build_time.Month,
    build_time.Day,build_time.Hour,build_time.Min,build_time.Sec);
    //BUILD:00000011223300
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
  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "YESTERDAY",(float)(g_rainfall.yesterday/10.f));
  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "1MIN", (float)(g_rainfall.min / 10.f));
  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "10MIN", (float)(g_rainfall.ten_min / 10.f));
  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "HOUR", (float)(g_rainfall.hourly / 10.f));
  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "TODAY", (float)(g_rainfall.today / 10.f));
  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "MONTH", (float)(g_rainfall.monthly / 10.f));
  screen_page_printf(p_win, "%-*s:%6.1f", RAIN_WD, "YEAR", (float)(g_rainfall.yearly / 10.f));
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
    screen_page_printf(p_win, "%-*s:%4.2f", CHARGER_WD, "SOLAR V",
                   read_solarVoltage1(&err));
    screen_page_printf(p_win, "%-*s:%4.2f", CHARGER_WD, "SOLAR A",
                   read_solarCurrent1(&err));
    screen_page_printf(p_win, "%-*s:%4.2f", CHARGER_WD, "BATTERY V",
                   read_batteryVoltage1(&err));
    screen_page_printf(p_win, "%-*s:%4.2f", CHARGER_WD, "LOAD A", read_loadCurrent1(&err));
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

#define CDMA_WD 6
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
                 ITEM_LIST(get_cdma_system()->link_status, link_status_list_eng));

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
                 ITEM_LIST(get_direct_system()->link_status, link_status_list_eng));

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
                   ITEM_LIST(get_tcp_client_system()->link_status, link_status_list_eng));

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
                     ITEM_LIST(link_status[i], ethlink_status_list_eng),
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
  const char *aws_title_list[] = {"CURR(AI)", "1MIN(AB)", "10MIN", "HOUR", "DAY","RAW"};
  char err_buf[32];
  char buffer[21];
  uint8_t err;
  float data, data_min=0, data_max=0;
  kma_data_ex_t *p_kma = NULL;
  uint8_t sensor_count=0;

  
  (void)data_min;
  (void)data_max;
  
  screen_page_start(p_win);

  
  
  if (min == eAWS_DATA_RAW)
  {
        screen_page_printf(p_win, "AWS %s %.2fs/%.2fs", aws_title_list[(int)min],
                           (float)g_exec_250ms_time.elapsed_time / 1000.0f,
                           (float)g_exec_1s_time.elapsed_time / 1000.0f);
  }
  else
  {
    snprintf(err_buf, sizeof(err_buf), "AWS %s", aws_title_list[(int)min]);
    make_centered(buffer,sizeof(buffer),err_buf,21);
    screen_page_printf(p_win, buffer);
  }

  p_kma = acquire_kma_data((eAWS_DATA_MIN_t)min);

  if(p_kma->updated==false)
  {
    screen_page_printf(p_win, "Data not updated yet");
    screen_page_clear(p_win);
    return;
  }

  if (p_kma->temperature.enable)
  {
    sensor_count++;
        err = p_kma->temperature.err;
    if (err)
    {
      data = p_kma->temperature.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "TEMP", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->temperature.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "TEMP", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->temperature.data);
        screen_page_printf(p_win, "%-*s:%6.1f C", AWS_WD, "TEMP", data);
      }
    }
  }
  // 상대습도
  if (p_kma->relative_humidity.enable)
  {
    sensor_count++;
    err = p_kma->relative_humidity.err;
    if (err)
    {
      data = p_kma->relative_humidity.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "HUMI", (int16_t)data, err_buf);
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
  // 풍향
  if (p_kma->wind_direction_avg.enable)
  {
    sensor_count++;
    err = p_kma->wind_direction_avg.err;
    if (err)
    {
      data = p_kma->wind_direction_avg.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "WIND D", (int16_t)data, err_buf);
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
    sensor_count++;
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      data = p_kma->wind_speed_avg.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "WIND S", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = ((int)(p_kma->wind_speed_avg.raw.f*100))/100.0;
        screen_page_printf(p_win, "%-*s:%6.2f m/s", AWS_WD, "WIND S", f_data);
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
      data = p_kma->wind_direction_instant.data;
      screen_page_printf(p_win, "%-*s:%04d E", AWS_WD, "WIND GD",(int16_t)data);
    }
    else
    {
      screen_page_printf(p_win, "%-*s:%6.1f deg", AWS_WD, "WIND GD",
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
      data = p_kma->wind_speed_instant.data;
      screen_page_printf(p_win, "%-*s:%04d E", AWS_WD, "WIND GS",(int16_t)data);
    }
    else
    {
      screen_page_printf(p_win, "%-*s:%6.1f m/s", AWS_WD, "WIND GS",
                     KMA_TO_GENERAL(p_kma->wind_speed_instant.data));
    }
  }

  // 강수량
  if (p_kma->precipitation.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    sensor_count++;
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
        screen_page_printf(p_win, "%-*s:%6.1f mm", AWS_WD, "RAIN(D)",
                       KMA_TO_GENERAL(p_kma->precipitation.data));
      }
    }
  }
  // 기압 //BAROMETER:1000.0hpa
  if (p_kma->pressure.enable)
  {
    sensor_count++;
    err = p_kma->pressure.err;
    if (err)
    {
      data = p_kma->pressure.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "BARO", (int16_t)data, err_buf);
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
    sensor_count++;
    err = p_kma->precipitation_presence.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%s", AWS_WD, "RAIN P", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW )
      {
        bool rain_p = p_kma->precipitation_presence.raw.b ;
        screen_page_printf(p_win, "%-*s: %s", AWS_WD, "RAIN P", rain_p?"ON":"OFF");
      }
      else if ( min == eAWS_DATA_REAL)
      {

        uint16_t data = p_kma->precipitation_presence.data;
        bool rain_p = (data == 10) ? true : false;
        int rain_off_remain_sec = (int)(g_rain_off_remain_time/1000.0);
        if(rain_p)
        {
          if (g_rain_timer_counting_down)
          {
            screen_page_printf(p_win, "%-*s: %s %03dsec", AWS_WD, "RAIN P","ON" , rain_off_remain_sec);
          }
          else
          {
            screen_page_printf(p_win, "%-*s: %s", AWS_WD, "RAIN P", "ON");
          }
        }
        else
        {
          screen_page_printf(p_win, "%-*s: %s", AWS_WD, "RAIN P",  "OFF");
        }

      }
      else
      {

        uint16_t data = p_kma->precipitation_presence.data;
        bool rain_p = (data == 10) ? true : false;
       // screen_page_printf(p_win, "%-*s: %4d", AWS_WD, "RAIN P",p_kma->precipitation_presence.data);
        screen_page_printf(p_win, "%-*s: %s", AWS_WD, "RAIN P", rain_p ? "ON" : "OFF");
      }
    }
  }
  // 적설
  if (p_kma->snowfall.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    sensor_count++;
    err = p_kma->snowfall.err;
    if (err)
    {
      data = p_kma->snowfall.data;

      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "SNOW", (int16_t)data, err_buf);
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


#define SOLAR_R_WD 7
  // 일사 "SOLAR R: 123.1 kW/m2"
  if (p_kma->solar_radiation.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    sensor_count++;
    err = p_kma->solar_radiation.err;
    if (err)
    {
     data = p_kma->solar_radiation.data ;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "SOLAR R", (int16_t)data, err_buf);
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
        case eAWS_DATA_REAL:
        {
          float solar_radiation = (float)g_solar_radiation.min_acc/1000.0f;
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
    sensor_count++;
    uint32_t solar_d_today = g_sunshine.today;

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
        }//SUNSHINE
        case eAWS_DATA_REAL:
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
    sensor_count++;
    err = p_kma->soil_temperature_5cm.err;
    if (err)
    {
      //
      data = p_kma->soil_temperature_5cm.data;

      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "ST 5cm",(int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_5cm.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
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
    sensor_count++;
    err = p_kma->soil_temperature_10cm.err;
    if (err)
    {
      data = p_kma->soil_temperature_10cm.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "ST 10cm", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_10cm.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
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
    sensor_count++;
    err = p_kma->soil_temperature_20cm.err;
    if (err)
    {
      data = p_kma->soil_temperature_20cm.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "ST 20cm", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_20cm.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
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
    sensor_count++;
    err = p_kma->soil_temperature_30cm.err;
    if (err)
    {
      data = p_kma->soil_temperature_30cm.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "ST 30cm", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_30cm.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
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
    sensor_count++;
    err = p_kma->soil_temperature_50cm.err;
    if (err)
    {
      data = p_kma->soil_temperature_50cm.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "ST 50cm", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_50cm.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
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
    sensor_count++;
    err = p_kma->soil_temperature_1m.err;
    if (err)
    {
      data = p_kma->soil_temperature_1m.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "ST 1m", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_1m.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
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
    sensor_count++;
    err = p_kma->soil_temperature_1_5m.err;
    if (err)
    {
      data = p_kma->soil_temperature_1_5m.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "ST 1.5m", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_1_5m.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
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
    sensor_count++;
    err = p_kma->soil_temperature_3m.err;
    if (err)
    {
      data = p_kma->soil_temperature_3m.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "ST 3m", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_3m.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
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
    sensor_count++;
    err = p_kma->soil_temperature_5m.err;
    if (err)
    {
      data = p_kma->soil_temperature_5m.data;
      make_error_string(err, err_buf, sizeof(err_buf));
      screen_page_printf(p_win, "%-*s:%04d %s", AWS_WD, "ST 5m", (int16_t)data, err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_5m.raw.f;
        if (less_float(f_data, 0) && bigger_float(f_data, -0.1))
        {
          f_data = 0;
        }
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

  if (sensor_count==0)
  {
    screen_page_printf(p_win, "No sensor configured");
    screen_page_printf(p_win, " ");
  }

    // 1분 자료인경우 상태표시
    if (min == eAWS_DATA_1MIN || min == eAWS_DATA_REAL)
    {

#define AWS_STATUS_WD 15

    sensor_t *p_sensor = get_sensor_config_copy();
    screen_page_printf(p_win, " ");

    make_centered(buffer, sizeof(buffer), "SENSOR STATUS", 21);
    screen_page_printf(p_win, buffer);


    for(int i = 0; i < 8; i++)
    {
    for (int bit = 0; bit < 8; bit++)
    {
      int sensor_index = i * 8 + bit;
      int bit_value = (p_kma->X_sensorStatus[i] >> bit) & 0x01;
      if (p_sensor[sensor_index].type==0)//미사용 센서는 표시 안함
      continue;

        if (sensor_index < _countof(sensor_name_eng_list))
        {
          if (bit_value)
          {
            screen_page_printf(p_win, "%-*s:FAIL", AWS_STATUS_WD, sensor_name_eng_list[sensor_index]);
          }
          else
          {
            screen_page_printf(p_win, "%-*s:NORM", AWS_STATUS_WD, sensor_name_eng_list[sensor_index]);
          }
        }
    }
    }
 



    uint8_t status_Y = p_kma->Y_volateStatus;
    const char *p_status;
    p_status = IS_BIT_SET(status_Y, 0)?"FAIL":"NORM";
    screen_page_printf(p_win, "%-*s:%s", AWS_STATUS_WD, "DC POWER", p_status);
    p_status = IS_BIT_SET(status_Y, 1) ? "FAIL" : "NORM";
    screen_page_printf(p_win, "%-*s:%s", AWS_STATUS_WD, "BATTERY", p_status);

    p_status = "????";
    if(IS_BIT_SET(status_Y, 2) && IS_BIT_SET(status_Y, 3))
    {
      p_status = "OFF";
    }
    else if (!IS_BIT_SET(status_Y, 2) && !IS_BIT_SET(status_Y, 3))
    {
      p_status = "110V";
    }
    else if (IS_BIT_SET(status_Y, 2) && !IS_BIT_SET(status_Y, 3))
    {
      p_status = "220V";
    }

    screen_page_printf(p_win, "%-*s:%s", AWS_STATUS_WD, "AC", p_status);
    p_status = IS_BIT_SET(status_Y, 4) ? "OPEN" : "CLOS";
    screen_page_printf(p_win, "%-*s:%s", AWS_STATUS_WD, "DOOR", p_status);
  }
    screen_page_clear(p_win);
 





}








void config_set_menu(void)
{

}



#define PAGE_CHARGER   0
#define PAGE_CDMA      1
#define PAGE_DIRECT    2
#define PAGE_ETH       3
#define PAGE_RAIN      4
#define PAGE_SYSTEM    5
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




void menu_refresh(void)
{
  if (g_menu_task_id)
    osThreadFlagsSet(g_menu_task_id, TASK_MENU_ALARM_UPDATE );
}

/**
 * @brief task_aws 에서 동기화 신호 발생시킴
 * 측정 task, 연산 task,표시 task가 제각각 주기로 동작중이라 실제 측정된 결과값이 표시되기까지
 * 지연이 발생됨
 * 따라서 표시 task는 측정 task에서 동기화 신호를 주어 즉각 표시 되도록 구현
 * 예)우량 신호를 발생 시켰는데 표시장치에는 1~2초 늦게 업데이트되는 현상방지 목적
 * 우량신호는 250ms마다 체크하여 최대 250ms~1000ms  
 */
void wait_refrech_trigger(void)
{
  osThreadFlagsWait(TASK_MENU_ALARM_UPDATE , osFlagsWaitAny, 250);
}


alert_t g_alert;

/**
 * @brief 다른 task에서 긴급 alert 화면을 뛰을때 사용
 * alert를 띄우고자하는 task는 미리 alert에 값을 쓴 상태에서 신호 발생해주면됨
 */
void show_alert(alert_t alert)
{
  g_alert  = alert;
  if (g_menu_task_id)
    osThreadFlagsSet(g_menu_task_id, TASK_MENU_ALARM_ALERT);
}

/**
 * @brief alert 신호가 있으면 alert변수에 저장된 값을 출력함
 * 오직 1개만 처리가능함
 */
void popup_handler(uint32_t timeout_ms)
{
  uint32_t flags;

  flags = osThreadFlagsWait(TASK_MENU_ALARM_ALERT , osFlagsWaitAny, 250);

  if (flags > 0  & flags&TASK_MENU_ALARM_ALERT)
  {
    show_popup(g_alert.title, (char*)g_alert.framebuffer);
    osThreadFlagsClear(TASK_MENU_ALARM_ALERT );
  }
}

void menuTask(void *arg)
{
  uint8_t first_page_done=1;
  int32_t key;
  int32_t page_count;//화면페이지 개수,config설정에 따라 자동 계산
  int32_t page_list[PAGE_MAX];//어떠한 페이지인지 저장
  uint32_t screen_off_time;
  screen_page_t lcd_win;

  DEBUG_PRINTF("menu task start\r\n");

  screen_page_create(&lcd_win);

  lcd_win.chunk_scroll_enable = 1;
  lcd_win.multi_page_enable = 1; 
  lcd_win.current_page = PAGE_SYSTEM;
  
  screen_off_time = OS_GET_TICK();
  while (1)
  {
    page_count = 0;

    if(get_config_app()->charger_model != eCHARGER_NONE)
    {
      page_list[page_count++] = PAGE_CHARGER;
    }
    if (get_config_app()->cdma_active)
      page_list[page_count++] = PAGE_CDMA;
    if (get_config_app()->direct_active)
      page_list[page_count++] = PAGE_DIRECT;
    if (get_config_app()->eth_active)
      page_list[page_count++] = PAGE_ETH;
    page_list[page_count++] = PAGE_RAIN;
    page_list[page_count++] = PAGE_SYSTEM;
    page_list[page_count++] = PAGE_AWS_AVG;
    page_list[page_count++] = PAGE_AWS_1MIN;
    // page_list[page_count++] = PAGE_AWS_10MIN; 구형에서는 표시 했지만 현재 불필요
    // page_list[page_count++] = PAGE_AWS_HOUR; 구형에서는 표시 했지만 현재 불필요
    page_list[page_count++] = PAGE_AWS_RAW;
    
    lcd_win.total_pages = page_count;

    if (first_page_done)//TODO:처음 부팅시 페이지 위치(개선 필요)
    {
      first_page_done = 0;
      for(int i =0; i < page_count;i++)
      {
        if (page_list[i] == PAGE_SYSTEM)//첫번째 페이지로 시스템 
        {
          lcd_win.current_page = i;
          break;
        }
      }
    }
      switch (page_list[lcd_win.current_page])
      {
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
      case PAGE_RAIN:
        draw_rain_page(&lcd_win);
        break;
      case PAGE_SYSTEM:
        draw_system_page(&lcd_win);
        break;
      case PAGE_AWS_AVG:
        draw_aws_page(&lcd_win, eAWS_DATA_REAL);
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

   // popup_handler(250);
    wait_refrech_trigger();//task_aws에서 값이 동기화신호 줌
    //

    key = get_button_key(0); 
    if (key == KEY_CODE_CTRL_C)
    {
      setup_menu();
      screen_off_time = OS_GET_TICK();//LCD off안되도록 갱신
    }
    else if (key == KEY_CODE_ESC_LONG)
    {
      int32_t choice = 0;
      if(input_active("Reset Device?",&choice)==MENU_OK)
      {
        if (choice)
        {
         // screen_off();
          screen_clear();
          screen_refresh();
          reset_system("key reset");
        }
      }
    }
    else if (key != KEY_CODE_NONE)
    {
       screen_page_handle(&lcd_win, key);
        screen_off_time = OS_GET_TICK(); // LCD off안되도록 갱신
    }

    if (get_config_app()->lcd_off_time_index != eLCD_OFF_ALWAYS_ON)
    {
      if ((OS_GET_TICK() - screen_off_time) > get_lcd_off_time() * OS_TICK_COUNT)
      {
        screen_off();

        key = get_button_key(0xFFFFFFFF); // 무한 대기
        screen_off_time = OS_GET_TICK();
        screen_on();
      }
    }

    }
}


bool g_boot_complete;

void set_boot_complete(void)
{
  g_boot_complete = true;
}

bool is_boot_complete(void)
{
  return g_boot_complete;
}

/*
화진 로고 밑에 
*표시하여 진행 상태 표시 
dual task에서 초기화 끝나면 부팅 완료 처리 
*/
void bootProgressTask(void *arg)
{
  char buffer[22];
  uint8_t count=0;

  screen_init();
  print_logo();

  while(1)
  {
    buffer[count++] = '*';
    buffer[count] = 0;

    screen_printf(6,0,buffer);
    if(count==screen_get_instance()->font_cols)
    {
      memset(buffer,' ',sizeof(buffer));
      buffer[sizeof(buffer)-1] = 0;
      screen_printf(6, 0, buffer);
      count = 0;
    }
    screen_refresh();
    osDelay(50);
    if(is_boot_complete()&&count ==0 )
    {
      break;
    }

  }

  g_menu_task_id =  osThreadNew(menuTask, NULL, &kMenuTask_attributes);
  osThreadExit(); // 종료 시킴
}


void menuTask_init(void)
{

//우선순가 높은 부팅이미지 출력 task먼저 실행하고  dual port task 초기화 되면 menu task 실행
  osThreadNew(bootProgressTask, NULL, &kBootProgressTask_attributes);


}

//
void menuPopUpTask(void *arg)
{
  while(1)
  {

  }
}

void menuPopUpTask_init(void)
{
  osThreadNew(menuPopUpTask, NULL, &kBootProgressTask_attributes);
}

void test_menu_info(void)
{
  screen_init();
  print_logo();

  test_setup_menu();
  while(1);
}