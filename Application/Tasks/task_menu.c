

#include "task_menu.h"
#include "cmsis_os2.h"
#include "app_lcd.h"


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


extern void st7920_flush_buffer(driver_t *drv);
void menuTask(void *arg)
{
  clcd_init();
  
  clcd_set_mode(eLCD_MODE_GRAPHIC);
  while (1)
  {
    clcd_clear();
    
    for(int row = 0;row<64;row++)
    {
      for(int col = 0 ; col < 128;col++)
      {
        clcd_set_pixel(col,row,1);
      }
    }



    
    clcd_flush_buffer();
#if 0 
    // clcd_write_string_at( 0,0,"123456789abcdefg");
    // clcd_write_string_at( 1,0,"23456789abcdefgh");
     clcd_write_string_at( 2,8,"3456789a");
       clcd_write_string_at( 3,8,"456789ab");
#endif
    osDelay(1000);  // 2초마다 페이지 전환
  }
}

void menuTask_init(void)
{


  osThreadNew(menuTask, NULL, &kMenuTask_attributes);
}



