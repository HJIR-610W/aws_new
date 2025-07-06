
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

const osThreadAttr_t kMenuTask_attributes = {
    .name = "menu",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityRealtime,
};

extern const char *doorStatusList[2];
extern const char *generalStatusList[2];

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
void menuTask_(void *arg)
{
  clcd_init();
  
  clcd_set_mode(eLCD_MODE_GRAPHIC);
  while (1)
  {
    clcd_clear();
    

    

    // Display sequential ASCII characters
    // 128 pixels / 6 pixels per character = 21 characters per row max
    // 64 pixels / 8 pixels per row = 8 rows max
    // Display only 21 characters per row for 8 rows
    uint8_t ascii_char = '0';  // Start with '0' (ASCII 48)
    
    for(int row = 0; row < 8; row++)
    {
      for(int col = 0; col < 21; col++)
      {
        clcd_put_ch(row, col, ascii_char);
        ascii_char++;
        
        // Wrap around if we go beyond printable ASCII
        if(ascii_char > 0x7E)
        {
          ascii_char = 0x20;  // Reset to space character
        }
      }
    }

    
    clcd_flush_buffer();

    osDelay(1000);  // 2초마다 페이지 전환
  }
}




#define SYSTEM_WD 10
void draw_system_page(lcd_win_t* win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];
	const char *message;

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "SYSTEM", LCD_COLS);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "%-*s: %d", SYSTEM_WD, "ID", get_config_app()->id);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "%-*s: %s", SYSTEM_WD, "DOOR",
	         ITEM_LIST(IS_DOOR_OPENED(), doorStatusList));
	lcd_print_row(win, row_count++, buff);
	
	if (get_logging_system()->status_group)
	{
		message = "ON";
	}
	else
	{
		message = "OFF";
	}
	
	snprintf(buff, sizeof(buff), "%-*s: %s", SYSTEM_WD, "LOGGING", message);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "%-*s: %.1f", SYSTEM_WD, "BATTERY V", bsp_read_battery());
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "%-*s: %.1f", SYSTEM_WD, "TEMP C", bsp_read_temperature());
	lcd_print_row(win, row_count++, buff);
	
	if (get_config_app()->ac_use)
	{
		snprintf(buff, sizeof(buff), "%-*s: %s", SYSTEM_WD, "AC", "ON");
		lcd_print_row(win, row_count++, buff);
	}
	
	win->total_items[page] = row_count;
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

	win->total_items[page] = row_count;
}

#define CHARGER_WD 15
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

	win->total_items[page] = row_count;
}

void draw_cdma_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "CDMA", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "STATUS: %s", "CONNECTED");
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "SIGNAL: %d", 85);
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] = row_count;
}

void draw_direct_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "DIRECT", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "STATUS: %s", "READY");
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] = row_count;
}

void draw_ethernet_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "ETHERNET", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "LINK: %s", "UP");
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "IP: 192.168.1.100");
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] = row_count;
}

void draw_aws_avg_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "AWS AVG", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "TEMP: %.1fC", 23.5f);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "HUMID: %.1f%%", 65.2f);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "WIND: %.1fm/s", 3.2f);
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] = row_count;
}

void draw_aws_1min_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "AWS 1MIN", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "TEMP: %.1fC", 23.8f);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "HUMID: %.1f%%", 64.8f);
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] = row_count;
}

void draw_aws_10min_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "AWS 10MIN", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "TEMP: %.1fC", 23.3f);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "HUMID: %.1f%%", 65.5f);
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] = row_count;
}

void draw_aws_hour_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "AWS HOUR", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "TEMP: %.1fC", 22.9f);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "HUMID: %.1f%%", 66.1f);
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] = row_count;
}

void draw_aws_raw_page(lcd_win_t *win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];

	win->current_row = 0;

	make_centered(buff, sizeof(buff), "AWS RAW", LCD_COLS);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year, Date_Time.Month,
	         Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "RAW1: %d", 1234);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "RAW2: %d", 5678);
	lcd_print_row(win, row_count++, buff);

	snprintf(buff, sizeof(buff), "RAW3: %d", 9012);
	lcd_print_row(win, row_count++, buff);

	win->total_items[page] = row_count;
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
	uint32_t last_update_time = 0;
	const uint32_t UPDATE_INTERVAL = 1000; // 1초마다 업데이트

	clcd_init();
	clcd_clear();
	
	lcd_create_win(&lcd_win);
	
	while (1)
	{
		// 페이지 목록 구성
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
		
		// 현재 시간 체크
		uint32_t current_time = osKernelGetTickCount();
		bool should_update = (current_time - last_update_time) >= UPDATE_INTERVAL;
		
		if (should_update)
		{
			lcd_clear_win(&lcd_win);
			
			// 현재 페이지에 따라 적절한 화면 그리기 함수 호출
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
					draw_aws_avg_page(&lcd_win);
					break;
				case PAGE_AWS_1MIN:
					draw_aws_1min_page(&lcd_win);
					break;
				case PAGE_AWS_10MIN:
					draw_aws_10min_page(&lcd_win);
					break;
				case PAGE_AWS_HOUR:
					draw_aws_hour_page(&lcd_win);
					break;
				case PAGE_AWS_RAW:
					draw_aws_raw_page(&lcd_win);
					break;
				default:
					break;
			}
			
			last_update_time = current_time;
		}
		
		// 키 입력 처리
		key = lcd_get_key_input();
		
		if (key == LCD_KEY_ESC)
		{
			config_set_menu();
		}
		else if (key != -1)
		{
			lcd_handle_scroll(&lcd_win, key);
			// 키 입력 시 즉시 화면 업데이트
			lcd_clear_win(&lcd_win);
			
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
					draw_aws_avg_page(&lcd_win);
					break;
				case PAGE_AWS_1MIN:
					draw_aws_1min_page(&lcd_win);
					break;
				case PAGE_AWS_10MIN:
					draw_aws_10min_page(&lcd_win);
					break;
				case PAGE_AWS_HOUR:
					draw_aws_hour_page(&lcd_win);
					break;
				case PAGE_AWS_RAW:
					draw_aws_raw_page(&lcd_win);
					break;
				default:
					break;
			}
		}
		
		osDelay(100); // 100ms 딜레이로 CPU 사용률 조절
	}
}

void menuTask_init(void)
{


  osThreadNew(menuTask, NULL, &kMenuTask_attributes);
}



