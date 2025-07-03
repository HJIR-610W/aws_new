#include "cli_key_code.h"
#include "cmsis_os2.h"
#include "dev_io.h"
#include "driver_lcd.h"
#include "cli_input.h"
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void test_lcd(void)
{
    driver_t *g_lcd_driver;
    uint8_t counter = 0;
    char display_str[17];
    char lcd_type[20];
    int lcd_driver_num = DRIVER_CLCD;
    
    const char *lcd_type_names[] = {"CLCD", "TERMINAL"};
    const int lcd_driver_nums[] = {DRIVER_CLCD, DRIVER_LCD_TERMNINAL};
    
    io_printf("LCD Test\r\n");
    io_printf("Select LCD Type: CLCD or TERMINAL\r\n");
#if 1
    if (cli_scanf_s("%19s", lcd_type) == CLI_KEYCODE_CTRL_C)
    {
        return;
    }
    
    for(int i = 0; i < 2; i++)
    {
        if(strcmp(lcd_type, lcd_type_names[i]) == 0)
        {
            lcd_driver_num = lcd_driver_nums[i];
            break;
        }
    }
    
    if(lcd_driver_num == -1)
    {
        io_printf("Invalid LCD Type. Please enter CLCD or TERMINAL.\r\n");
        return;
    }
#endif
    g_lcd_driver = driver_lcd_open(lcd_driver_num);
    if(g_lcd_driver == NULL)
    {
        io_printf("LCD driver open failed\r\n");
        return;
    }
    
    driver_lcd_display_on(g_lcd_driver);
    
    io_printf("LCD Test Start (%s) - Press CTRL+Q to exit\r\n", lcd_type);
    

                    
                    
    if(lcd_driver_num == DRIVER_CLCD)
    {
        while (1)
        {
            driver_lcd_set_mode(g_lcd_driver, eLCD_MODE_CHARACTER);
            driver_lcd_clear_screen(g_lcd_driver);
            
            io_printf("Character mode test (10 seconds)\r\n");
            
            for(int i = 0; i < 10; i++)
            {
              
                
                for(int j = 0; j < 16; j++)
                {
                    display_str[j] = '0' + j;
                }
                display_str[16] = '\0';
                
                for(int row = 0; row < 4; row++)
                {
                    driver_lcd_set_position(g_lcd_driver, 0, row);
                    driver_lcd_write_string(g_lcd_driver, display_str);
                }
                
                counter++;
                if(counter > 9)
                {
                    counter = 0;
                }
                
                if (get_key(1000) == KEY_CODE_CTRL_Q)
                {
                    goto exit_test;
                }
            }
            driver_lcd_set_mode(g_lcd_driver, eLCD_MODE_GRAPHIC);
            driver_lcd_clear_screen(g_lcd_driver);
            
            io_printf("Drawing sin graph\r\n");
            
            for(int x = 0; x < 128; x++)
            {
                float angle = (float)x * 2.0f * M_PI / 128.0f;
                float sin_val = sinf(angle);
                
                int y = 32 + (int)(sin_val * 20.0f);
                
                if(y < 0) y = 0;
                if(y > 63) y = 63;
                
                driver_lcd_set_pixel(g_lcd_driver, x, y, true);
            }
            
            for(int i = 0; i < 5; i++)
            {
                if (get_key(1000) == KEY_CODE_CTRL_Q)
                {
                    goto exit_test;
                }
            }
        }
    }
    else
    {
        while (1)
        {
            driver_lcd_clear_screen(g_lcd_driver);
            
            for(int i = 0; i < 16; i++)
            {
                display_str[i] = '0' + counter;
            }
            display_str[16] = '\0';
            
            for(int row = 0; row < 4; row++)
            {
                driver_lcd_set_position(g_lcd_driver, 0, row);
                driver_lcd_write_string(g_lcd_driver, display_str);
            }
            
            counter++;
            if(counter > 9)
            {
                counter = 0;
            }
            
            if (get_key(1000) == KEY_CODE_CTRL_Q)
            {
                break;
            }
        }
    }

exit_test:
    
    driver_lcd_clear_screen(g_lcd_driver);
    
    io_printf("LCD Test End\r\n");
}