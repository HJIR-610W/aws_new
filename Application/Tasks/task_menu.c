

#include "task_menu.h"
#include "cmsis_os2.h"
#include "app_lcd.h"
#include "../App_drivers/lcd/font_16x8.h"
#include "../App_drivers/lcd/font_6x8.h"
#include "app_button.h"

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





void menuTask(void *arg)
{
  int32_t key;
  char display_char[2] = {0, 0}; // Single character string with null terminator
  
  clcd_init();
  app_button_init(); // Initialize button system
  
  clcd_set_mode(eLCD_MODE_GRAPHIC);
  
  // Clear screen initially
  clcd_clear();
  clcd_write_string_at(0, 0, "Ready for input:");
    
  while(1)
  {
    key = get_button_key(100); // Check for keys every 100ms
    
    // Check if key is a lowercase alphabet letter (a-z)
    if (key >= 'a' && key <= 'z') {
      // Clear the display area first
      clcd_clear();
      
      // Convert key to character and display at row 0, col 0
      display_char[0] = (char)key;
      clcd_write_string_at(0, 0, display_char);
      
      // Optional: Add some feedback
      clcd_write_string_at(1, 0, "Key pressed!");
    }
    else if (key != KEY_CODE_NONE) {
      // Optional: Handle other keys or show feedback
      clcd_clear();
      clcd_write_string_at(0, 0, "Not lowercase");
    }
    
    // Small delay to prevent excessive CPU usage
    osDelay(10);
  }
}

void menuTask_init(void)
{


  osThreadNew(menuTask, NULL, &kMenuTask_attributes);
}



