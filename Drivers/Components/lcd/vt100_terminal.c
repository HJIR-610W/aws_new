/**
 * @file vt100_terminal.c
 * @brief VT100 터미널 에뮬레이터 (io_printf/io_puts 사용)
 */

#include "dev_io.h"
#include "driver_lcd_define.h"
#include "driver_interface.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "driver_uart.h"

/* VT100 터미널 기본 크기 */
#define VT100_DEFAULT_ROWS    4
#define VT100_DEFAULT_COLS    20

/* VT100 이스케이프 시퀀스 */
#define VT100_CLEAR_SCREEN    "\033[2J"     // 화면 지우기
#define VT100_CURSOR_HOME     "\033[H"      // 커서 홈으로
#define VT100_CURSOR_SHOW     "\033[?25h"   // 커서 표시
#define VT100_CURSOR_HIDE     "\033[?25l"   // 커서 숨기기
#define VT100_CURSOR_POS      "\033[%d;%dH" // 커서 위치 설정


typedef struct {
    driver_t *uart_io;
    bool initialized;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t max_rows;
    uint8_t max_cols;
} vt100_terminal_t;

static vt100_terminal_t vt100_instance;
static driver_t vt100_driver;



void vt100_io_pirntf(driver_t *drv,const char *pFmt, ...)
{
    vt100_terminal_t *vt100 = (vt100_terminal_t*)(drv->cfg);
    
    if(!vt100->initialized || pFmt == NULL) return;
    
    char buffer[266];  // 충분한 버퍼 크기
    va_list args;
    
    va_start(args, pFmt);
    int len = vsnprintf(buffer, sizeof(buffer), pFmt, args);
    va_end(args);
    
    if(len > 0 && len < sizeof(buffer))
    {
        driver_uart_send(vt100->uart_io, (uint8_t*)buffer, len);
    }
}

void vt100_io_puts(driver_t *drv,char *string)
{
  vt100_terminal_t *vt100 = (vt100_terminal_t *)(drv->cfg);

  driver_uart_send(vt100->uart_io, (uint8_t *)string, strlen(string));
}
// VT100 터미널용 LCD API 함수들
static void vt100_set_position(driver_t *drv, uint8_t x, uint8_t y)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
       
    if(x < term->max_cols && y < term->max_rows)
    {
        term->cursor_x = x;
        term->cursor_y = y;
        
        // VT100 커서 위치 설정 명령 전송
        vt100_io_pirntf(drv, "\x1B[%d;%dH", y + 1, x + 1);

    
    }
}

static void vt100_write_string(driver_t *drv, const char *str)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
    
    if(!term->initialized || str == NULL) return;
    

    vt100_io_pirntf(drv, "%s", str);


}


static void vt100_clear_screen(driver_t *drv)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
    
    if(!term->initialized) return;
    
    // VT100 화면 지우기 및 커서 홈으로
    vt100_io_puts(drv, "\x1B[2J\x1B[f");

    term->cursor_x = 0;
    term->cursor_y = 0;
}

static void vt100_home(driver_t *drv)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
    
    if(!term->initialized) return;
    
    // VT100 커서 홈으로
    vt100_io_puts(drv, "\033[H");

    term->cursor_x = 0;
    term->cursor_y = 0;
}

static void vt100_display_on(driver_t *drv)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
    
    if(!term->initialized) return;
    
    // VT100 화면 켜기 (커서 표시)
    vt100_io_puts(drv, "\033[?25h");
}

static void vt100_display_off(driver_t *drv)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
    
    if(!term->initialized) return;
    
    // VT100 화면 끄기 (커서 숨기기)
    vt100_io_puts(drv,"\033[?25l");
}

static void vt100_write_string_at(driver_t *drv, int row, int col, const char *str)
{
    if (!str || row < 0 || col < 0) return;
    
    // 위치 설정 후 문자열 출력
    vt100_set_position(drv, row, col);
    vt100_write_string(drv, str);
}

// VT100 터미널 LCD API 구조체
static lcd_api_t vt100_lcd_api = {
    .set_position = vt100_set_position,
    .write_string = vt100_write_string,
    .write_string_at = vt100_write_string_at,
    .clear_screen = vt100_clear_screen,
    .home = vt100_home,
    .display_on = vt100_display_on,
    .display_off = vt100_display_off
};

driver_t* vt100_terminal_open(void)
{
  uart_config_t uart_config;

    if(vt100_driver.opened)
    {
        return &vt100_driver;
    }

  uart_config.baud = 115200;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

        // VT100 터미널 초기화
    vt100_instance.initialized = true;
    vt100_instance.cursor_x = 0;
    vt100_instance.cursor_y = 0;
    vt100_instance.max_rows = 24;  // 표준 터미널 크기
    vt100_instance.max_cols = 80;
    vt100_instance.uart_io = driver_uart_open(UART_8_CDMA,&uart_config);
    vt100_driver.cfg = &vt100_instance;
    vt100_driver.api = &vt100_lcd_api;
    vt100_driver.opened = true;
    
    // VT100 터미널 초기화 시퀀스
    vt100_io_puts(&vt100_driver ,"\033[2J");  // 화면 지우기
    vt100_io_puts(&vt100_driver ,"\033[H");    // 커서 홈으로
    vt100_io_puts(&vt100_driver ,"\033[?25h");  // 커서 표시

    return &vt100_driver;
}