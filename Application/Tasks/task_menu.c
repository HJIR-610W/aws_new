
#include <stdio.h>
#include "task_menu.h"
#include "cmsis_os2.h"
#include "app_lcd.h"
#include "../App_drivers/lcd/font_16x8.h"
#include "../App_drivers/lcd/font_6x8.h"
#include "app_button.h"
#include "lcd_driver.h"

#include "util_time.h"

const osThreadAttr_t kMenuTask_attributes = {
    .name = "menu",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityBelowNormal,
};
int display_page = 0;
const char *menu_lines[4] = {
    "123456789abcdefg",
    "23456789abcdefgh", 
    "3456789abcdefghi",
    "456789abcdefghij"
};


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




void draw_water_page(lcd_win_t* win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];
	static int water_level = 85;
	static float water_flow = 12.5f;
	static float water_temp = 22.3f;
	static float water_ph = 7.2f;
	static float water_turbidity = 1.2f;
	static float water_pressure = 2.1f;

	

	
  win->current_row = 0;
	

	
	// 각 행별 데이터 출력
	snprintf(buff, sizeof(buff), "Level: %d%%", water_level);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "Flow: %.1fL/min", water_flow);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "Temp: %.1fC", water_temp);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "pH: %.2f", water_ph);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "Turbidity: %.1fNTU", water_turbidity);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "Pressure: %.1fbar", water_pressure);
	lcd_print_row(win, row_count++, buff);

  
  	snprintf(buff, sizeof(buff), "Press5ure: %.1fbar", water_pressure);
	lcd_print_row(win, row_count++, buff);
  
  	snprintf(buff, sizeof(buff), "Pres4sure: %.1fbar", water_pressure);
	lcd_print_row(win, row_count++, buff);
  
  	snprintf(buff, sizeof(buff), "Pres3sure: %.1fbar", water_pressure);
	lcd_print_row(win, row_count++, buff);
  
  	snprintf(buff, sizeof(buff), "Pr2essure: %.1fbar", water_pressure);
	lcd_print_row(win, row_count++, buff);
	// 페이지별 데이터 설정
	win->total_items[page] = row_count;
}

void draw_system_page(lcd_win_t* win)
{
	int row_count = 0;
	int page = win->current_page;
	char buff[LCD_COLS + 1];
	static int water_level = 85;
	static float water_flow = 12.5f;
	static float water_temp = 22.3f;
	static float water_ph = 7.2f;
	static float water_turbidity = 1.2f;
	static float water_pressure = 2.1f;

        win->current_row = 0;

        // LCD 화면 클리어

	
	// 각 행별 데이터 출력

	lcd_print_row(win, row_count++, "SYTEM");
	
	snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", Date_Time.Year,Date_Time.Month,
Date_Time.Day,Date_Time.Hour,Date_Time.Min,Date_Time.Sec);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "1Temp: %.1fC", water_temp);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "1pH: %.2f", water_ph);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "Tur1bidity: %.1fNTU", water_turbidity);
	lcd_print_row(win, row_count++, buff);
	
	snprintf(buff, sizeof(buff), "Pres1sure: %.1fbar", water_pressure);
	lcd_print_row(win, row_count++, buff);

	// 페이지별 데이터 설정
	win->total_items[page] = row_count;
}


void menuTask(void *arg)
{
  
  int32_t key;
		lcd_win_t  lcd_win;
  
  
  clcd_init();

  
 // clcd_set_mode(eLCD_MODE_GRAPHIC);
  

  clcd_clear();
  clcd_write_string_at(0, 0, "Ready for input:");
  
  
  	lcd_create_win(&lcd_win);
    
    	lcd_win.total_pages = 2;
      
      

	while (1) {
		// 현재 페이지에 따라 데이터 표시
		if (lcd_win.current_page == 0) {
			draw_system_page(&lcd_win);
		} else {
			draw_water_page(&lcd_win);
		}
    
    		// 키 입력 처리
		int key = lcd_get_key_input();
		
		if (key == LCD_KEY_ESC) {
			break; // 프로그램 종료
		} else if(key !=-1){
			// 모든 키 입력을 스크롤/페이지 핸들러로 전달
			lcd_handle_scroll(&lcd_win, key);
		}
    
    
  }
}

void menuTask_init(void)
{


  osThreadNew(menuTask, NULL, &kMenuTask_attributes);
}



