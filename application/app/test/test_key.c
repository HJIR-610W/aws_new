

#include "app_key.h"
#include "debug_io.h"
#include "menu_handler.h"

void test_key(void)
{
  int32_t key;
  app_key_init();
  
  
  while(1)
  {
    scan_key();
    key = get_menu_key(0);
    
    if(key !=KEY_CODE_NONE)
    {
  
    switch (key)
        {
        case KEY_CODE_LEFT:
        debug_printf("LEFT\r\n");
          break;
        case KEY_CODE_RIGHT:
        debug_printf("RIGHT\r\n");
          break;
        case KEY_CODE_UP:
        debug_printf("UP\r\n");
          break;
        case KEY_CODE_DOWN:
        debug_printf("DOWN\r\n");
          break;
        case KEY_CODE_CTRL_C:
        debug_printf("ESC\r\n");
          break;
        case KEY_CODE_ENTER:
        debug_printf("ENTER\r\n");
          break;
        default:
          debug_printf("%c\r\n",key);
          break;
        }
    }

        if (debug_get_key(100) == KEY_CODE_CTRL_C)
    {
      break;
    }
    
  }
}