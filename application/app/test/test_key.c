

#include "app_key.h"
#include "dev_io.h"

void test_key(void)
{
  int32_t key;
  app_key_init();
  
  
  while(1)
  {
    scan_key();
    key = get_button_key(0);
    
    if(key !=KEY_CODE_NONE)
    {
  
    switch (key)
        {
        case KEY_CODE_LEFT:
        io_printf("LEFT\r\n");
          break;
        case KEY_CODE_RIGHT:
        io_printf("RIGHT\r\n");
          break;
        case KEY_CODE_UP:
        io_printf("UP\r\n");
          break;
        case KEY_CODE_DOWN:
        io_printf("DOWN\r\n");
          break;
        case KEY_CODE_CTRL_C:
        io_printf("ESC\r\n");
          break;
        case KEY_CODE_ENTER:
        io_printf("ENTER\r\n");
          break;
        default:
          io_printf("%c\r\n",key);
          break;
        }
    }

    
    if (get_key(100) == KEY_CODE_CTRL_Q)
    {
      break;
    }
    
  }
}