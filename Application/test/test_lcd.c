#include "cli_key_code.h"
#include "cmsis_os2.h"
#include "dev_io.h"
#include "driver_lcd.h"
#include "cli_input.h"
#include <string.h>

void test_lcd(void)
{
    driver_t *g_lcd_driver;
    uint8_t counter = 0;
    char display_str[17]; // 16문자 + NULL
    char lcd_type[20];
    int lcd_driver_num = -1;
    
    const char *lcd_type_names[] = {"CLCD", "TERMINAL"};
    const int lcd_driver_nums[] = {DRIVER_CLCD, DRIVER_LCD_TERMNINAL};
    
    io_printf("LCD 테스트\r\n");
    io_printf("LCD 타입을 선택하세요: CLCD 또는 TERMINAL\r\n");
    
    // 사용자로부터 LCD 타입 입력받기
    if (cli_scanf_s("%19s", lcd_type) == CLI_KEYCODE_CTRL_C)
    {
        return;
    }
    
    // 입력된 타입과 매칭
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
        io_printf("잘못된 LCD 타입입니다. CLCD 또는 TERMINAL을 입력하세요.\r\n");
        return;
    }
    
    // LCD 드라이버 열기
    g_lcd_driver = driver_lcd_open(lcd_driver_num);
    if(g_lcd_driver == NULL)
    {
        io_printf("LCD driver open failed\r\n");
        return;
    }
    
    // LCD 디스플레이 켜기
    driver_lcd_display_on(g_lcd_driver);
    
    io_printf("LCD Test Start (%s) - Press CTRL+Q to exit\r\n", lcd_type);
    
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