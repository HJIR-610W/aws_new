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
#include "util_escape_sequence.h"


#define VT100_DEFAULT_ROWS    8
#define VT100_DEFAULT_COLS    20

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
static uint8_t framebuffer[VT100_DEFAULT_ROWS][VT100_DEFAULT_COLS];  


void vt100_flush_buffer(driver_t *drv);
void vt100_put_ch(driver_t *drv, int row, int col, uint8_t ch);

void vt100_io_pirntf(driver_t *drv,const char *pFmt, ...)
{
    vt100_terminal_t *vt100 = (vt100_terminal_t*)(drv->cfg);
    
    if(!vt100->initialized || pFmt == NULL) return;

    char buffer[VT100_DEFAULT_COLS*2];  // 충분한 버퍼 크기
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
static void vt100_set_position(driver_t *drv, uint8_t row, uint8_t col)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
       
    if(col < term->max_cols && row < term->max_rows)
    {
        term->cursor_x = col;
        term->cursor_y = row;
        
        // VT100 커서 위치 설정 명령 전송
        vt100_io_pirntf(drv, "\x1B[%d;%dH", row + 1, col + 1);

    
    }
}



static void vt100_clear_screen(driver_t *drv)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
    
    if(!term->initialized) return;

    vt100_io_puts(drv, ES_CLEAR_SCREEN);
    vt100_io_puts(drv, ES_CURSOR_HOME);

    term->cursor_x = 0;
    term->cursor_y = 0;
}

static void vt100_home(driver_t *drv)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
    
    if(!term->initialized) return;
    
    vt100_io_puts(drv, ES_CURSOR_HOME);

    term->cursor_x = 0;
    term->cursor_y = 0;
}

static void vt100_display_on(driver_t *drv)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
    
    if(!term->initialized) return;

    vt100_io_puts(drv, ES_CLEAR_SCREEN);
}

static void vt100_display_off(driver_t *drv)
{
    vt100_terminal_t *term = (vt100_terminal_t *)drv->cfg;
    
    if(!term->initialized) return;

    vt100_io_puts(drv, ES_CLEAR_SCREEN);
}

static void vt100_write_string_at(driver_t *drv, int row, int col, const char *str)
{
    if (!str || row < 0 || col < 0) return;
    
    // 위치 설정 후 문자열 출력
    vt100_set_position(drv, row, col);
    vt100_io_puts(drv,(char *)str);
}

// VT100 터미널 LCD API 구조체
static lcd_api_t vt100_lcd_api = {.set_position = vt100_set_position,
                                  .write_string_at = vt100_write_string_at,
                                  .clear_screen = vt100_clear_screen,
                                  .home = vt100_home,
                                  .display_on = vt100_display_on,
                                  .display_off = vt100_display_off,
                                  .flush = vt100_flush_buffer,
                                  .put_ch = vt100_put_ch};

driver_t* vt100_terminal_open(void)
{
  uart_config_t uart_config;

  if(vt100_driver.opened)
  {
      return &vt100_driver;
  }

  uart_config.baud = 115200;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;
  uart_config.dataLen = UART_DATA_LEN_8;


  vt100_instance.initialized = true;
  vt100_instance.cursor_x = 0;
  vt100_instance.cursor_y = 0;
  vt100_instance.max_rows = VT100_DEFAULT_ROWS;  // 표준 터미널 크기
  vt100_instance.max_cols = VT100_DEFAULT_COLS;
  vt100_instance.uart_io = driver_uart_open(UART_0_D_SUB_0,&uart_config);
  vt100_driver.cfg = &vt100_instance;
  vt100_driver.api = &vt100_lcd_api;
  vt100_driver.opened = true;
    

  vt100_io_puts(&vt100_driver, ES_CLEAR_SCREEN); 
  vt100_io_puts(&vt100_driver, ES_CURSOR_HOME);
  vt100_io_puts(&vt100_driver, ES_CURSOR_OFF);

  return &vt100_driver;
}


void vt100_flush_buffer(driver_t *drv)
{
  char buff[VT100_DEFAULT_COLS+1];

  for (int row=0; row < VT100_DEFAULT_ROWS; row++)
  {
    vt100_set_position(drv, row, 0);
    strncpy(buff, (char *)framebuffer[row], VT100_DEFAULT_COLS);
    buff[VT100_DEFAULT_COLS] = 0;
    vt100_io_puts(&vt100_driver, buff);
  }
}

void vt100_put_ch(driver_t *drv, int row, int col, uint8_t ch)
{
    framebuffer[row][col] = ch;
}