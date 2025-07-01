#include "cli_key_code.h"
#include "cmsis_os2.h"
#include "dev_io.h"
#include "driver_lcd.h"

void test_lcd(void)
{
    driver_t *g_lcd_driver;
    uint8_t counter = 0;
    char display_str[17]; // 16문자 + NULL
    
    // LCD 드라이버 열기
    g_lcd_driver = driver_lcd_open(DRIVER_LCD);
    if(g_lcd_driver == NULL)
    {
        io_printf("LCD driver open failed\r\n");
        return;
    }
    
    // LCD 디스플레이 켜기
    driver_lcd_display_on(g_lcd_driver);
    
    io_printf("LCD Test Start - Press CTRL+Q to exit\r\n");
    
    while (1)
    {
        // 화면 지우기
        driver_lcd_clear_screen(g_lcd_driver);
        
        // 0~9까지 16문자로 채우기
        for(int i = 0; i < 16; i++)
        {
            display_str[i] = '0' + counter;
        }
        display_str[16] = '\0';
        
        // 모든 행(0~3)에 같은 숫자 출력
        for(int row = 0; row < 4; row++)
        {
            driver_lcd_set_position(g_lcd_driver, 0, row);
            driver_lcd_write_string(g_lcd_driver, display_str);
        }
        
        // 카운터 증가 (0~9 순환)
        counter++;
        if(counter > 9)
        {
            counter = 0;
        }
        
        // CTRL+Q 체크 (1초 대기하면서)
        if (get_key(1000) == KEY_CODE_CTRL_Q)
        {
            break;
        }
    }
    
    // 종료시 화면 지우기
    driver_lcd_clear_screen(g_lcd_driver);
    
    // 종료 메시지
    io_printf("LCD Test End\r\n");
}