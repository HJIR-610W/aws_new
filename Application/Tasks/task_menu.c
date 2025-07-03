

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

void menuTask(void *arg)
{
  clcd_init();
  
  while (1)
  {
    clcd_clear();
    
     clcd_write_string_at( 0,0,"123456789abcdefg");
     clcd_write_string_at( 1,0,"23456789abcdefgh");
     clcd_write_string_at( 2,0,"3456789abcdefghi");
       clcd_write_string_at( 3,0,"456789abcdefghij");
    osDelay(1000);  // 2초마다 페이지 전환
  }
}

void menuTask_init(void)
{


  osThreadNew(menuTask, NULL, &kMenuTask_attributes);
}



