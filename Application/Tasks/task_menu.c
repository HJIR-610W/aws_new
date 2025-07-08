
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "task_menu.h"
#include "cmsis_os2.h"
#include "app_lcd.h"
#include "../App_drivers/lcd/font_16x8.h"
#include "../App_drivers/lcd/font_6x8.h"
#include "app_button.h"
#include "lcd_driver.h"
#include "util_time.h"
#include "config_app.h"
#include "console_utile.h"
#include "task_logging.h"
#include "bsp_di.h"
#include "bsp.h"
#include "aws_data.h"
#include "app_charger.h"
#include "task_cellular.h"
#include "util_time.h"
#include "time_define.h"
#include "task_direct.h"
#include "task_tcpServer.h"
#include "task_client.h"
#include "tcp_define.h"

#include "view_driver.h"
#include "task_measure.h"
const osThreadAttr_t kMenuTask_attributes = {
    .name = "menu",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};

extern exec_time_t g_exec_250ms_time;  // Task 실행 시간 측정용
extern exec_time_t g_exec_1s_time;            // Task 실행 시간 측정용
extern void make_error_string(uint8_t error, char *buffer, uint32_t buffer_size);
extern const char *linkStatusList[3];
extern const char *generalStatusList[2];
const char *doorStatusList_lcd[2]={"CLOSED","OPENED"};
const char *linkStatusList_lcd[3] = {"-", "UP", "DOWN"};
const char *ethlinkStatusList_lcd[3] = {"-", "U", "D"};




int display_page = 0;
const char *menu_lines[4] = {
    "123456789abcdefg",
    "23456789abcdefgh", 
    "3456789abcdefghi",
    "456789abcdefghij"
};

void make_centered(char *buffer, size_t buf_size, const char *text, int width)
{
  int text_len = strlen(text);


  if (text_len >= width || buf_size <= 1)
  {
    snprintf(buffer, buf_size, "%.*s", (int)buf_size - 1, text);
    return;
  }
  int left_padding = (width - text_len) / 2;
  int written = snprintf(buffer, buf_size, "%*s%s", left_padding, "", text);
  if (written < 0 || written >= buf_size - 1)
  {
    return;
  }

  for (int i = written; i < width && i < buf_size - 1; i++)
  {
    buffer[i] = ' ';
  }
  int end_pos = (width < buf_size) ? width : (int)buf_size - 1;
  buffer[end_pos] = '\0';
}

void draw_char_h(int start_x, int start_y)
{
    // 'h' character is at index 72 in the font table (0x68 - 0x20)
    const uint8_t *char_data = font_16x8[72];
    
    for (int y = 0; y < FONT_HEIGHT; y++)
    {
        uint8_t row_data = char_data[y];
        for (int x = 0; x < FONT_WIDTH; x++)
        {
            if (row_data & (0x80 >> x))  // Check bit from MSB
            {
                clcd_set_pixel(start_x + x, start_y + y, 1);
            }
        }
    }
}

void draw_char_p_6x8(int start_x, int start_y)
{
    // 'p' character is at index 80 in the font table (0x70 - 0x20)
    const uint8_t *char_data = font_6x8[80];
    
    for (int y = 0; y < FONT_6X8_HEIGHT; y++)
    {
        uint8_t row_data = char_data[y];
        for (int x = 0; x < FONT_6X8_WIDTH; x++)
        {
            if (row_data & (0x10 >> x))  // Check bit from bit 4 (6-bit font uses bits 4-0)
            {
                clcd_set_pixel(start_x + x, start_y + y, 1);
            }
        }
    }
}

extern void st7920_flush_buffer(driver_t *drv);







#define SYSTEM_WD 10
void draw_system_page(lcd_win_t* p_win)
{
	int row_count = 0;
	int page = p_win->current_page;
	char buff[LCD_COLS + 1];
	const char *message;

	p_win->current_row = 0;

	make_centered(buff, sizeof(buff), "SYSTEM", LCD_COLS);
	lcd_print_row(p_win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(p_win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "%-*s: %d", SYSTEM_WD, "ID", get_config_app()->id);
	lcd_print_row(p_win, row_count++, buff);

        snprintf(buff, sizeof(buff), "%-*s: %s", SYSTEM_WD, "DOOR",
                 ITEM_LIST(IS_DOOR_OPENED(), doorStatusList_lcd));
        lcd_print_row(p_win, row_count++, buff);
	
	if (get_logging_system()->status_group)
	{
		message = "ERROR";
	}
	else
	{
		message = "NORMAL";
	}
	
	snprintf(buff, sizeof(buff), "%-*s: %s", SYSTEM_WD, "LOGGING", message);
	lcd_print_row(p_win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "%-*s: %.1f", SYSTEM_WD, "SYS VOLT", bsp_read_battery());
	lcd_print_row(p_win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "%-*s: %.1f", SYSTEM_WD, "SYS TEMP", bsp_read_temperature());
	lcd_print_row(p_win, row_count++, buff);
	
	if (get_config_app()->ac_use)
	{
		snprintf(buff, sizeof(buff), "%-*s: %s", SYSTEM_WD, "AC", "ON");
		lcd_print_row(p_win, row_count++, buff);
	}
	
	p_win->total_items[page] =  ALIGN_UP(row_count, p_win->current_row ); 
  
  
    while (p_win->current_row < p_win->view_row)
  {
     lcd_print_row(p_win, row_count++, "                    ");
  }

  
  
}

#define RAIN_WD 8
void draw_rain_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "RAIN", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "YESTERDAY", get_rainfall()->rainfall_yesterday);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "TODAY", get_rainfall()->rainfall_today);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "1MIN", get_rainfall()->rainfall_1min);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "10MIN", get_rainfall()->rainfall_10min);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "HOUR", get_rainfall()->rainfall_hourly);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "YEAR", get_rainfall()->rainfall_yearly);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %6.1f", SYSTEM_WD, "MONTH", get_rainfall()->rainfall_monthly);
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] =  ALIGN_UP(row_count, win->current_row ); 
  
  
    while (win->current_row < win->view_row)
  {
     lcd_print_row(win, row_count++, "                    ");
  }

}

#define CHARGER_WD 10
void draw_charger_page(lcd_win_t *win)
{
	uint8_t err;
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];
	char temp[20];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "CHARGER", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	read_chargerStatus(temp, sizeof(temp));
	snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "STATUS", temp);
	lcd_print_row(win, row_count++, buff);

	if (is_chargerValid())
	{
		snprintf(buff, sizeof(buff), "%-*s: %.2f", CHARGER_WD, "SOLAR V",
		         read_solarVoltage1(&err));
		lcd_print_row(win, row_count++, buff);

		snprintf(buff, sizeof(buff), "%-*s: %.2f", CHARGER_WD, "SOLAR A",
		         read_solarCurrrent1(&err));
		lcd_print_row(win, row_count++, buff);

		snprintf(buff, sizeof(buff), "%-*s: %.2f", CHARGER_WD, "BATTERY V",
		         read_batteryVoltage1(&err));
		lcd_print_row(win, row_count++, buff);

		snprintf(buff, sizeof(buff), "%-*s: %.2f", CHARGER_WD, "LOAD A",
		         read_loadCurrent1(&err));
		lcd_print_row(win, row_count++, buff);
	}
	else
	{
		snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "SOLAR V", "-");
		lcd_print_row(win, row_count++, buff);

		snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "SOLAR A", "-");
		lcd_print_row(win, row_count++, buff);

		snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "BATTERY V", "-");
		lcd_print_row(win, row_count++, buff);

		snprintf(buff, sizeof(buff), "%-*s: %s", CHARGER_WD, "LOAD A", "-");
		lcd_print_row(win, row_count++, buff);
	}

	win->total_items[page] =  ALIGN_UP(row_count, win->current_row ); 
  
    while (win->current_row < win->view_row)
  {
     lcd_print_row(win, row_count++, "                    ");
  }

}

#define CDMA_WD 15
void draw_cdma_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];
	char num[20];
	DATE_TIME_BUF nt;
	uint32_t last_time;

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "CDMA", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %s", CDMA_WD, "LINK",
	         ITEM_LIST(get_cdma_system()->link_status, linkStatusList));
	lcd_print_row(win, row_count++, buff);

	if (get_cdma_system()->num[0] != '0')
	{
		num[0] = '-';
		num[1] = 0;
	}
	else
	{
		snprintf(num, sizeof(num), "%s", get_cdma_system()->num);
	}
	snprintf(buff, sizeof(buff), "%-*s: %s", CDMA_WD, "PHONE", num);
	lcd_print_row(win, row_count++, buff);

	if (get_cdma_system()->rssi == -1)
	{
		snprintf(buff, sizeof(buff), "%-*s: -", CDMA_WD, "RSSI");
	}
	else
	{
		snprintf(buff, sizeof(buff), "%-*s: %d", CDMA_WD, "RSSI", get_cdma_system()->rssi);
	}
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %d", CDMA_WD, "TX", get_cdma_system()->tx_cnt);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %d", CDMA_WD, "RX", get_cdma_system()->rx_cnt);
	lcd_print_row(win, row_count++, buff);

	last_time = get_cdma_system()->last_recv_time;
	if (last_time == 0)
	{
		snprintf(buff, sizeof(buff), "%-*s: -", CDMA_WD, "R TIME");
	}
	else
	{
		time_cvt_secTotime(last_time, &nt);
		snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", CDMA_WD, "R TIME",
		         nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
	}
	lcd_print_row(win, row_count++, buff);

	last_time = get_cdma_system()->last_send_time;
	if (last_time == 0)
	{
		snprintf(buff, sizeof(buff), "%-*s: -", CDMA_WD, "T TIME");
	}
	else
	{
		time_cvt_secTotime(last_time, &nt);
		snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", CDMA_WD, "T TIME",
		         nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
	}
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] =  ALIGN_UP(row_count, win->current_row ); 
  
      while (win->current_row < win->view_row)
  {
     lcd_print_row(win, row_count++, "                    ");
  }
}

#define DIRECT_WD 8
void draw_direct_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];
	DATE_TIME_BUF nt;
	uint32_t last_time;
	uint32_t remain_sec;

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "DIRECT", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %s", DIRECT_WD, "LINK",
	         ITEM_LIST(get_direct_system()->link_status, linkStatusList));
	lcd_print_row(win, row_count++, buff);

	remain_sec = (uint32_t)(get_direct_system()->linkdown_remain_ms / 1000.0);
	snprintf(buff, sizeof(buff), "%-*s: %ds", DIRECT_WD, "TIMEOUT", remain_sec);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %d", DIRECT_WD, "TX", get_direct_system()->tx_cnt);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%-*s: %d", DIRECT_WD, "RX", get_direct_system()->rx_cnt);
	lcd_print_row(win, row_count++, buff);

	last_time = get_direct_system()->last_recv_time;
	if (last_time == 0)
	{
		snprintf(buff, sizeof(buff), "%-*s: -", DIRECT_WD, "R TIME");
	}
	else
	{
		time_cvt_secTotime(last_time, &nt);
		snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", DIRECT_WD, "R TIME",
		         nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
	}
	lcd_print_row(win, row_count++, buff);

	last_time = get_direct_system()->last_send_time;
	if (last_time == 0)
	{
		snprintf(buff, sizeof(buff), "%-*s: -", DIRECT_WD, "T TIME");
	}
	else
	{
		time_cvt_secTotime(last_time, &nt);
		snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", DIRECT_WD, "T TIME",
		         nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
	}
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] =  ALIGN_UP(row_count, win->current_row ); 
}

//:192.168.123.123
#define ETH_WD 2
void draw_ethernet_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];
	DATE_TIME_BUF nt;
	eLINK_STATUS_t link_status[ETH_CLIENT_MAX];
	uint8_t tx_cnt[ETH_CLIENT_MAX];
	uint8_t rx_cnt[ETH_CLIENT_MAX];
	uint32_t last_time;

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "ETHERNET", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	if (get_config_app()->eth_mode == eETH_MODE_CLINET)
	{
		snprintf(buff, sizeof(buff), "%-*s: %s", ETH_WD, "LINK",
		         ITEM_LIST(get_tcp_client_system()->link_status, linkStatusList_lcd));
		lcd_print_row(win, row_count++, buff);

		snprintf(buff, sizeof(buff), "%-*s: %d", ETH_WD, "TX", get_tcp_client_system()->tx_cnt);
		lcd_print_row(win, row_count++, buff);

		snprintf(buff, sizeof(buff), "%-*s: %d", ETH_WD, "RX", get_tcp_client_system()->rx_cnt);
		lcd_print_row(win, row_count++, buff);

		last_time = get_tcp_client_system()->last_recv_time;
		if (last_time == 0)
		{
			snprintf(buff, sizeof(buff), "%-*s: -", ETH_WD, "RT");
		}
		else
		{
			time_cvt_secTotime(last_time, &nt);
			snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", ETH_WD, "RT",
			         nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
		}
		lcd_print_row(win, row_count++, buff);

		last_time = get_tcp_client_system()->last_send_time;
		if (last_time == 0)
		{
			snprintf(buff, sizeof(buff), "%-*s: -", ETH_WD, "TT");
		}
		else
		{
			time_cvt_secTotime(last_time, &nt);
			snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", ETH_WD, "TT",
			         nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
		}
		lcd_print_row(win, row_count++, buff);
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
			snprintf(buff, sizeof(buff), "L%d:%s(%s)", i,
			         ITEM_LIST(link_status[i], ethlinkStatusList_lcd),
			         get_tcp_system(i)->client_ip_str);
			lcd_print_row(win, row_count++, buff);

			snprintf(buff, sizeof(buff), "%-*s: %d", ETH_WD, "TX", tx_cnt[i]);
			lcd_print_row(win, row_count++, buff);

			snprintf(buff, sizeof(buff), "%-*s: %d", ETH_WD, "RX", rx_cnt[i]);
			lcd_print_row(win, row_count++, buff);

			last_time = get_tcp_system(i)->last_recv_time;
			if (last_time == 0)
			{
				snprintf(buff, sizeof(buff), "%-*s: -", ETH_WD, "RT");
			}
			else
			{
				time_cvt_secTotime(last_time, &nt);
				snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", ETH_WD, "RT",
				         nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
			}
			lcd_print_row(win, row_count++, buff);

			last_time = get_tcp_system(i)->last_send_time;
			if (last_time == 0)
			{
				snprintf(buff, sizeof(buff), "%-*s: -", ETH_WD, "TT");
			}
			else
			{
				time_cvt_secTotime(last_time, &nt);
				snprintf(buff, sizeof(buff), "%-*s: %02d-%02d-%02d %02d:%02d:%02d", ETH_WD, "TT",
				         nt.Year % 100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
			}
			lcd_print_row(win, row_count++, buff);
		}
	}

	win->total_items[page] =  ALIGN_UP(row_count, win->current_row ); 
}

#define AWS_WD 6
void draw_aws_avg_page(lcd_win_t *p_win, eAWS_DATA_MIN_t min)
{
  const char *aws_title_list[] = {"AVG", "1MIN", "10MIN", "HOUR", "RAW"};
  char buff[LCD_COLS + 1];
  char err_buf[32];
  uint8_t err;
  int row_count = 0;
  int page = p_win->current_page;
  float data, data_min, data_max;
  kma_data_ex_t *p_kma = NULL;

  p_win->current_row = 0;

  snprintf(buff, sizeof(buff), "AWS %s %.2fs/%.2fs", aws_title_list[(int)min],
           (float)g_exec_250ms_time.elapsed_time / 1000.0f,
           (float)g_exec_1s_time.elapsed_time / 1000.0f);

  lcd_print_row(p_win, row_count++, buff);

  p_kma = get_kma_data((eAWS_DATA_MIN_t)min);

  if (p_kma->temperature.enable)
	{
		err = p_kma->temperature.err;
		if (err)
		{
			make_error_string(err, err_buf, sizeof(err_buf));
			snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "TEMP", err_buf);
		}
		else
		{
				float f_data = p_kma->temperature.raw.f;
				snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "TEMP", f_data);

		}
    lcd_print_row(p_win, row_count++, buff);
  }

  // 풍향
  if (p_kma->wind_direction_avg.enable)
  {
    err = p_kma->wind_direction_avg.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "WIND D", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->wind_direction_avg.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f", AWS_WD, "WIND D", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->wind_direction_avg.data);
        data_max = KMA_TO_GENERAL(p_kma->wind_direction_avg.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f", AWS_WD, "WIND D", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 풍속
  if (p_kma->wind_speed_avg.enable)
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "WIND S", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->wind_speed_avg.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f m/s", AWS_WD, "WIND S", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->wind_speed_avg.data);
        data_max = KMA_TO_GENERAL(p_kma->wind_speed_avg.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f m/s", AWS_WD, "WIND S", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 순간 풍향
  if (p_kma->wind_direction_avg.enable &&
      (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR && page != eAWS_DATA_RAW))
  {
    err = p_kma->wind_direction_avg.err;
    if (err)
    {
      snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "GUST WIND D");
    }
    else
    {
      snprintf(buff, sizeof(buff), "%-*s: %7.1f", AWS_WD, "GUST WIND D",
               KMA_TO_GENERAL(p_kma->wind_direction_instant.data));
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 순간 풍속
  if (p_kma->wind_speed_avg.enable &&
      (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR && page != eAWS_DATA_RAW))
  {
    err = p_kma->wind_speed_avg.err;
    if (err)
    {
      snprintf(buff, sizeof(buff), "%-*s: --", AWS_WD, "GUST WIND S");
    }
    else
    {
      snprintf(buff, sizeof(buff), "%-*s: %7.1f m/s", AWS_WD, "GUST WIND S",
               KMA_TO_GENERAL(p_kma->wind_speed_instant.data));
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 강수량
  if (p_kma->precipitation.enable && (min != eAWS_DATA_10MIN && min != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "RAIN", err_buf);
    }
    else
    {
      if (min == eAWS_DATA_RAW)
      {
        uint32_t last_time = p_kma->precipitation.last_time;
        DATE_TIME_BUF nt;

        if (last_time == 0)
        {
          snprintf(buff, sizeof(buff), "%-*s:--", AWS_WD, "RAIN(t)");
        }
        else
        {
          time_cvt_secTotime(last_time, &nt);
					//RAIN(t):250101000000
          snprintf(buff, sizeof(buff), "%-*s:%02d%02d%02d%02d%02d%02d", AWS_WD, "RAIN(t)",
                   nt.Year%100, nt.Month, nt.Day, nt.Hour, nt.Min, nt.Sec);
        }
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s:%6.1f mm", AWS_WD, "RAIN",
                 KMA_TO_GENERAL(p_kma->precipitation.data));
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }
  // 기압
  if (p_kma->pressure.enable)
  {
    err = p_kma->pressure.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "BARO", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->pressure.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f hPa", AWS_WD, "BARO", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->pressure.data);
        data_min = KMA_TO_GENERAL(p_kma->pressure.min);
        data_max = KMA_TO_GENERAL(p_kma->pressure.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f hPa", AWS_WD, "BARO", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 강수유무
  if (p_kma->precipitation_presence.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->precipitation_presence.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "RAIN_P", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->precipitation_presence.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %5d", AWS_WD, "RAIN_P", (int)f_data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %5d", AWS_WD, "RAIN_P",
                 p_kma->precipitation_presence.data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }
  // 적설
  if (p_kma->snowfall.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->snowfall.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SNOW", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        int data = (int)p_kma->snowfall.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7d mm", AWS_WD, "SNOW", data);
      }
      else
      {
        snprintf(buff, sizeof(buff), "%-*s: %7d mm", AWS_WD, "SNOW", p_kma->snowfall.data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 상대습도
  if (p_kma->relative_humidity.enable)
  {
    err = p_kma->relative_humidity.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "HUMI", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->relative_humidity.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f %%", AWS_WD, "HUMI", f_data);
      }
      else
      {
        data = KMA_TO_GENERAL(p_kma->relative_humidity.data);
        data_min = KMA_TO_GENERAL(p_kma->relative_humidity.min);
        data_max = KMA_TO_GENERAL(p_kma->relative_humidity.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f %%", AWS_WD,"HUMI", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 일사
  if (p_kma->solar_radiation.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->solar_radiation.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOLAR R", err_buf);
    }
    else
    {
      switch (page)
      {
        case eAWS_DATA_RAW:
        {
          float f_data = p_kma->solar_radiation.raw.f;
          snprintf(buff, sizeof(buff), "%-*s: %7.2f WJ/m2", AWS_WD, "SOLAR R", f_data);
          break;
        }
        case eAWS_DATA_AVG:
        {
          float solar_radiation = p_kma->solar_radiation.data;
          snprintf(buff, sizeof(buff), "%-*s: %7.1f WJ/m2", AWS_WD, "SOLAR R", solar_radiation);
          break;
        }
        default:
        {
          float solar_radiation = p_kma->solar_radiation.data;
          snprintf(buff, sizeof(buff), "%-*s: %7.1f KJ/m2", AWS_WD, "SOLAR R", solar_radiation);
          break;
        }
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 일조
  if (p_kma->sunshine_duration.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->sunshine_duration.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOLAR D", err_buf);
    }
    else
    {
      switch (page)
      {
        case eAWS_DATA_RAW:
        {
          float f_data = p_kma->sunshine_duration.raw.f;
          bool sunshine_duration = (f_data == 1.0f);
          snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOLAR D",
                   sunshine_duration ? "ON" : "OFF");
          break;
        }
        case eAWS_DATA_AVG:
        {
          bool sunshine_duration = (p_kma->sunshine_duration.data == 1);
          snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOLAR D",
                   sunshine_duration ? "ON" : "OFF");
          break;
        }
        default:
          snprintf(buff, sizeof(buff), "%-*s: %5d s", AWS_WD, "SOLAR D",
                   p_kma->sunshine_duration.data);
          break;
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 지중온도 5cm
  if (p_kma->soil_temperature_5cm.enable)
  {
    err = p_kma->soil_temperature_5cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOIL T 5cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_5cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "SOIL T 5cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD,
                 "SOIL T 5cm", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 지중온도 10cm
  if (p_kma->soil_temperature_10cm.enable)
  {
    err = p_kma->soil_temperature_10cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOIL T 10cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_10cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "SOIL T 10cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD,"SOIL T 10cm", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 지중온도 20cm
  if (p_kma->soil_temperature_20cm.enable)
  {
    err = p_kma->soil_temperature_20cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOIL T 20cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_20cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "SOIL T 20cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD,"SOIL T 20cm", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 지중온도 30cm
  if (p_kma->soil_temperature_30cm.enable)
  {
    err = p_kma->soil_temperature_30cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOIL T 30cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_30cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "SOIL T 30cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD,"SOIL T 30cm", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }
  // 지중온도 50cm
  if (p_kma->soil_temperature_50cm.enable)
  {
    err = p_kma->soil_temperature_50cm.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOIL T 50cm", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_50cm.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "SOIL T 50cm", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD,
                 "SOIL T 50cm", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 지중온도 1m
  if (p_kma->soil_temperature_1m.enable)
  {
    err = p_kma->soil_temperature_1m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOIL T 1m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_1m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "SOIL T 1m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD, "SOIL T 1m",  data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 지중온도 1.5m
  if (p_kma->soil_temperature_1_5m.enable)
  {
    err = p_kma->soil_temperature_1_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOIL T 1.5m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_1_5m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "SOIL T 1.5m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD,"SOIL T 1.5m", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 지중온도 3m
  if (p_kma->soil_temperature_3m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_temperature_3m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOIL T 3m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_3m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "SOIL T 3m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD, "SOIL T 3m", data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  // 지중온도 5m
  if (p_kma->soil_temperature_5m.enable && (page != eAWS_DATA_10MIN && page != eAWS_DATA_HOUR))
  {
    err = p_kma->soil_temperature_5m.err;
    if (err)
    {
      make_error_string(err, err_buf, sizeof(err_buf));
      snprintf(buff, sizeof(buff), "%-*s: %s", AWS_WD, "SOIL T 5m", err_buf);
    }
    else
    {
      if (page == eAWS_DATA_RAW)
      {
        float f_data = p_kma->soil_temperature_5m.raw.f;
        snprintf(buff, sizeof(buff), "%-*s: %7.2f C", AWS_WD, "SOIL T 5m", f_data);
      }
      else
      {
        data = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.data);
        data_min = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.min);
        data_max = KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.max);
        snprintf(buff, sizeof(buff), "%-*s: %7.1f C", AWS_WD, "SOIL T 5m",data);
      }
    }
    lcd_print_row(p_win, row_count++, buff);
  }

  p_win->total_items[page] =  ALIGN_UP(row_count, p_win->current_row ); 
  
  while (p_win->current_row < p_win->view_row)
  {
     lcd_print_row(p_win, row_count++, "                    ");
  }




}








void config_set_menu(void)
{

}

#define PAGE_SYSTEM 0
#define PAGE_RAIN 1
#define PAGE_CHARGER 2
#define PAGE_CDMA 3
#define PAGE_DIRECT 4
#define PAGE_ETH       5
#define PAGE_AWS_AVG   6
#define PAGE_AWS_1MIN  7
#define PAGE_AWS_10MIN 8
#define PAGE_AWS_HOUR  9
#define PAGE_AWS_RAW   10

void menuTask(void *arg)
{
	int32_t key;
	lcd_win_t lcd_win;
	int32_t page_count = 0;
	int32_t page_list[15];


	clcd_init();
  
  clcd_set_mode(eLCD_MODE_GRAPHIC);
  

	
	lcd_create_win(&lcd_win);
	
  
	while (1)
  {
		page_count = 0;
		page_list[page_count++] = PAGE_SYSTEM;
		page_list[page_count++] = PAGE_RAIN;
		page_list[page_count++] = PAGE_CHARGER;
		
		if (get_config_app()->cdma_use)
		{
			page_list[page_count++] = PAGE_CDMA;
		}
		
		if (get_config_app()->direct_use)
		{
			page_list[page_count++] = PAGE_DIRECT;
		}
		
		if (get_config_app()->eth_use)
		{
			page_list[page_count++] = PAGE_ETH;
		}
		
		page_list[page_count++] = PAGE_AWS_AVG;
		page_list[page_count++] = PAGE_AWS_1MIN;
		page_list[page_count++] = PAGE_AWS_10MIN;
		page_list[page_count++] = PAGE_AWS_HOUR;
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
                    draw_aws_avg_page(&lcd_win, eAWS_DATA_AVG);
                    break;
                  case PAGE_AWS_1MIN:
                    draw_aws_avg_page(&lcd_win, eAWS_DATA_1MIN);
                    break;
                  case PAGE_AWS_10MIN:
                    draw_aws_avg_page(&lcd_win, eAWS_DATA_10MIN);
                    break;
                  case PAGE_AWS_HOUR:
                    draw_aws_avg_page(&lcd_win, eAWS_DATA_HOUR);
                    break;
                  case PAGE_AWS_RAW:
                    draw_aws_avg_page(&lcd_win, eAWS_DATA_RAW);
                    break;
                  default:
                    break;
                }
           
                clcd_flush_buffer();
						
                key = lcd_get_key_input(1000);
		
		if (key == LCD_KEY_ESC)
		{
			config_set_menu();
		}
		else if (key != -1)
		{
			lcd_handle_scroll(&lcd_win, key);

	
		}
		

	}
}

void menuTask_init(void)
{


  osThreadNew(menuTask, NULL, &kMenuTask_attributes);
}



