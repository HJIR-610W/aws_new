/**
 * @file st7920.c
 * @brief ST7920 128x64 그래픽 LCD 드라이버 (시리얼 인터페이스)
 */

#include "st7920.h"
#include <string.h>
#include "driver_spi.h"
#include "driver_stm32_do.h"
#include "driver_do.h"
#include "pcb_define.h"
#include "driver_lcd_define.h"
#include "mcu_utile.h"
#include "usDelay.h"

#define ST7920_WIDTH 128
#define ST7920_HEIGHT 64

/* ST7920 기본 명령어 */
#define ST7920_CMD_DISPLAY_CLEAR 0x01
#define ST7920_CMD_RETURN_HOME 0x02
#define ST7920_CMD_ENTRY_MODE_SET 0x04
#define ST7920_CMD_DISPLAY_CONTROL 0x08
#define ST7920_CMD_CURSOR_SHIFT 0x10
#define ST7920_CMD_FUNCTION_SET 0x20
#define ST7920_CMD_SET_CGRAM_ADDR 0x40
#define ST7920_CMD_SET_DDRAM_ADDR 0x80

/* 기능 설정 비트 */
#define ST7920_FUNCTION_SET_8BIT 0x10
#define ST7920_FUNCTION_SET_EXTEND 0x04   // 확장 명령 세트 활성화
#define ST7920_FUNCTION_SET_GRAPHIC 0x02  // 그래픽 모드 활성화

#define ST7920_DISPLAY_ON 0x04
#define ST7920_CURSOR_ON 0x02
#define ST7920_BLINK_ON 0x01

/* 시리얼 인터페이스 동기화 바이트 - ST7920은 3바이트 시퀀스로 통신 */
#define ST7920_SYNC_BYTE 0xF8
#define ST7920_SYNC_CMD 0xFA   // 명령 전송시 첫 바이트
#define ST7920_SYNC_DATA 0xFE  // 데이터 전송시 첫 바이트

typedef struct
{
  driver_t *spi_io;
  driver_t *cs_io;  // CS는 active high
  driver_t *rst_io;
  bool initialized;
  bool graphic_mode;
} st7920_t;



static st7920_t st7920_instance;
static driver_t st7920_driver;
static uint8_t framebuffer[ST7920_HEIGHT][ST7920_WIDTH / 8];  // 그래픽 모드용 프레임버퍼

lcd_api_t lcd_api = {
    .set_position = st7920_set_position,
    .write_string = st7920_write_string,
    .clear_screen = st7920_clear_screen,
    .home = st7920_home,
    .display_on = st7920_display_on,
    .display_off = st7920_display_off
};


inline void st7920_delay_ms(uint32_t ms)
{

  osDelay(ms);
  
}

inline void st7920_delay_us(uint32_t us_delay)
{
  usDelay(us_delay);
}

driver_t *st7920_open(void)
{
    if(st7920_driver.opened)
    {
        return &st7920_driver;
    }

    st7920_instance.spi_io = driver_spi_open(STM_SPI_1);
    if(!st7920_instance.spi_io)
    {
        return NULL;
    }

    st7920_instance.cs_io = driver_do_open(DO_FLASH_CS, NULL);
    if(!st7920_instance.cs_io)
    {
        return NULL;
    }

    st7920_instance.rst_io = driver_do_open(DO_HART_RESET, NULL);

    st7920_instance.initialized = false;
    st7920_instance.graphic_mode = false;
    
    st7920_driver.cfg = &st7920_instance;
    st7920_driver.opened = true;
    st7920_driver.api = &lcd_api;

    st7920_reset(&st7920_driver);
    
    return &st7920_driver;
}

void st7920_reset(driver_t *drv)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    // 하드웨어 리셋 시퀀스
    driver_do_high(cfg->cs_io);
    st7920_delay_ms(10);
    driver_do_low(cfg->cs_io);
    st7920_delay_ms(50);
    
    // ST7920 초기화 시퀀스 - 3번 반복으로 안정화
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_ms(5);
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_us(100);
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_us(100);
    
    st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CONTROL);
    st7920_delay_us(100);
    st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CLEAR);  // 1.6ms 필요
    st7920_delay_ms(2);
    st7920_send_cmd(drv, ST7920_CMD_ENTRY_MODE_SET | 0x02);  // 커서 증가, 시프트 없음
    st7920_delay_us(100);
    
    cfg->initialized = true;
}

void st7920_send_byte(driver_t *drv, uint8_t sync, uint8_t data)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    // CS active high로 통신 시작
    driver_do_high(cfg->cs_io);
    st7920_delay_us(1);
    
    // ST7920 시리얼 프로토콜: 동기바이트 + 상위4비트 + 하위4비트
    driver_spi_send_byte(cfg->spi_io, sync);
    driver_spi_send_byte(cfg->spi_io, data & 0xF0);
    driver_spi_send_byte(cfg->spi_io, (data << 4) & 0xF0);
    
    st7920_delay_us(1);
    driver_do_low(cfg->cs_io);  // 통신 종료
    st7920_delay_us(100);  // 명령 처리 시간 확보
}

void st7920_send_cmd(driver_t *drv, uint8_t cmd)
{
    st7920_send_byte(drv, ST7920_SYNC_CMD, cmd);
    st7920_delay_us(100);
}

void st7920_send_data(driver_t *drv, uint8_t data)
{
    st7920_send_byte(drv, ST7920_SYNC_DATA, data);
    st7920_delay_us(100);
}

void st7920_clear_screen(driver_t *drv)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    if(cfg->graphic_mode)
    {
        memset(framebuffer, 0, sizeof(framebuffer));
        
        // 그래픽 메모리는 상하 2개 영역으로 분할됨
        // 상단 32줄 (Y=0~31, X=0~7)
        for(int y = 0; y < 32; y++)
        {
            st7920_send_cmd(drv, 0x80 | y);  // Y 주소
            st7920_send_cmd(drv, 0x80);      // X 주소 (상단 영역)
            for(int x = 0; x < 16; x++)
            {
                st7920_send_data(drv, 0x00);
            }
        }
        
        // 하단 32줄 (Y=32~63, X=8~15)
        for(int y = 0; y < 32; y++)
        {
            st7920_send_cmd(drv, 0x80 | y);  // Y 주소
            st7920_send_cmd(drv, 0x88);      // X 주소 (하단 영역)
            for(int x = 0; x < 16; x++)
            {
                st7920_send_data(drv, 0x00);
            }
        }
    }
    else
    {
        st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CLEAR);
        st7920_delay_ms(2);  // 클리어 명령은 1.6ms 필요
    }
}

void st7920_home(driver_t *drv)
{
    st7920_send_cmd(drv, ST7920_CMD_RETURN_HOME);
    st7920_delay_ms(2);
}

void st7920_display_on(driver_t *drv)
{
    st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CONTROL | ST7920_DISPLAY_ON);
    st7920_delay_us(100);
}

void st7920_display_off(driver_t *drv)
{
    st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CONTROL);
    st7920_delay_us(100);
}

void st7920_set_graphic_mode(driver_t *drv, bool enable)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    if(enable)
    {
        // 1단계: 확장 명령 세트 활성화
        st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT | ST7920_FUNCTION_SET_EXTEND);
        st7920_delay_us(100);
        // 2단계: 그래픽 모드 활성화
        st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT | 
                           ST7920_FUNCTION_SET_EXTEND | ST7920_FUNCTION_SET_GRAPHIC);
        st7920_delay_us(100);
        cfg->graphic_mode = true;
    }
    else
    {
        // 기본 명령 세트로 복귀
        st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
        st7920_delay_us(100);
        cfg->graphic_mode = false;
    }
}

void st7920_set_position(driver_t *drv, uint8_t x, uint8_t y)
{
    uint8_t addr = 0x80;
    
    if(y >= 2)
    {
        addr += 0x20;
        y -= 2;
    }
    
    if(y == 1)
    {
        addr += 0x10;
    }
    
    addr += x;
    st7920_send_cmd(drv, addr);
    st7920_delay_us(100);
}

void st7920_write_string(driver_t *drv, const char *str)
{
    while(*str)
    {
        st7920_send_data(drv, *str++);
    }
}

void st7920_set_pixel(driver_t *drv, uint8_t x, uint8_t y, bool on)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    if(x >= ST7920_WIDTH || y >= ST7920_HEIGHT || !cfg->graphic_mode)
        return;
    
    uint8_t byte_x = x / 8;
    uint8_t bit_x = x % 8;
    
    // 프레임버퍼 업데이트
    if(on)
    {
        framebuffer[y][byte_x] |= (0x80 >> bit_x);
    }
    else
    {
        framebuffer[y][byte_x] &= ~(0x80 >> bit_x);
    }
    
    // ST7920 그래픽 메모리 매핑: 128x64 = 상단32줄(0~7) + 하단32줄(8~15)
    uint8_t row = y;
    uint8_t col = byte_x;
    
    if(row >= 32)  // 하단 영역
    {
        row -= 32;
        col += 8;
    }
    
    st7920_send_cmd(drv, 0x80 | row);  // Y 주소 설정
    st7920_send_cmd(drv, 0x80 | col);  // X 주소 설정
    st7920_send_data(drv, framebuffer[y][byte_x]);
}

void st7920_draw_line(driver_t *drv, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool on)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int x = x1, y = y1;
    
    while(1)
    {
        st7920_set_pixel(drv, x, y, on);
        
        if(x == x2 && y == y2)
            break;
            
        int e2 = 2 * err;
        if(e2 > -dy)
        {
            err -= dy;
            x += sx;
        }
        if(e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }
}

void st7920_draw_rect(driver_t *drv, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool fill, bool on)
{
    if(fill)
    {
        for(uint8_t i = 0; i < height; i++)
        {
            for(uint8_t j = 0; j < width; j++)
            {
                st7920_set_pixel(drv, x + j, y + i, on);
            }
        }
    }
    else
    {
        st7920_draw_line(drv, x, y, x + width - 1, y, on);
        st7920_draw_line(drv, x + width - 1, y, x + width - 1, y + height - 1, on);
        st7920_draw_line(drv, x + width - 1, y + height - 1, x, y + height - 1, on);
        st7920_draw_line(drv, x, y + height - 1, x, y, on);
    }
}

void st7920_draw_bitmap(driver_t *drv, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    if(!cfg->graphic_mode)
        return;
    
    for(uint8_t row = 0; row < height; row++)
    {
        for(uint8_t col = 0; col < width; col += 8)
        {
            uint8_t byte_data = bitmap[(row * ((width + 7) / 8)) + (col / 8)];
            
            for(uint8_t bit = 0; bit < 8 && (col + bit) < width; bit++)
            {
                if(byte_data & (0x80 >> bit))
                {
                    st7920_set_pixel(drv, x + col + bit, y + row, true);
                }
            }
        }
    }
}

/*
// ST7920 LCD 사용 예제

 텍스트 모드 사양:
  - 행 수: 4행 (0~3)
  - 열 수: 16열 (0~15)
  - 총 문자: 64문자
  - 문자 크기: 8×16 픽셀

  주소 매핑:
  - 1행 (y=0): 0x80~0x8F
  - 2행 (y=1): 0x90~0x9F
  - 3행 (y=2): 0x88~0x97
  - 4행 (y=3): 0x98~0xA7

void st7920_example(void)
{
    driver_t *lcd = st7920_init();
    if(!lcd) return;

    // 디스플레이 켜기
    st7920_display_on(lcd);

    // 텍스트 모드에서 문자열 출력
    st7920_clear_screen(lcd);

    // 0행 0열에 "hello" 출력
    st7920_set_position(lcd, 0, 0);
    st7920_write_string(lcd, "hello");

    // 1행 0열에 "test" 출력
    st7920_set_position(lcd, 0, 1);
    st7920_write_string(lcd, "test");

    // 그래픽 모드 예제
    st7920_set_graphic_mode(lcd, true);
    st7920_clear_screen(lcd);

    // 사각형 그리기
    st7920_draw_rect(lcd, 10, 10, 50, 30, false, true);

    // 대각선 그리기
    st7920_draw_line(lcd, 0, 0, 127, 63, true);

    // 픽셀 찍기
    st7920_set_pixel(lcd, 64, 32, true);
}
*/