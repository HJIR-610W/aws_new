/**
 * @file st7920.c
 * @brief ST7920 128x64 그래픽 LCD 드라이버 (시리얼 인터페이스)
 */

#include "st7920.h"
#include <string.h>
#include "driver_stm32_spi.h"
#include "driver_stm32_do.h"
#include "driver_do.h"
#include "pcb_define.h"
#include "driver_lcd_define.h"
#include "mcu_utile.h"
#include "usDelay.h"
#include <math.h>
#include <stdlib.h>
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

//확장 명령어
#define ST7920_CMD_SET_SR 0x02

/* 기능 설정 비트 */
#define ST7920_FUNCTION_SET_8BIT 0x10
#define ST7920_FUNCTION_SET_EXTEND 0x04   // 확장 명령 세트 활성화
#define ST7920_FUNCTION_SET_GRAPHIC 0x02  // 그래픽 모드 활성화

#define ST7920_DISPLAY_ON 0x04
#define ST7920_CURSOR_ON 0x02
#define ST7920_BLINK_ON 0x01


#define ST7920_SYNC_CMD  0xF8   // 명령 전송시 첫 바이트
#define ST7920_SYNC_DATA 0xFA  // 데이터 전송시 첫 바이트

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

static void st7920_set_mode(driver_t *drv, eLCD_MODE_t lcd_mode);
static void st7920_write_string_api(driver_t *drv, const char *str);

lcd_api_t lcd_api = {
    .set_position = st7920_set_position,
    .write_string = st7920_write_string_api,
    .write_string_at = st7920_write_string,
    .clear_screen = st7920_clear_screen,
    .home = st7920_home,
    .display_on = st7920_display_on,
    .display_off = st7920_display_off,
    .set_mode = st7920_set_mode,
    .set_pixel = st7920_set_pixel,
    .draw_line = st7920_draw_line
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

    st7920_instance.cs_io = driver_do_open(DO_LCD_CS, NULL);
    if(!st7920_instance.cs_io)
    {
        return NULL;
    }

    st7920_instance.rst_io = driver_do_open(DO_LCD_RESET, NULL);

    st7920_instance.initialized = false;
    st7920_instance.graphic_mode = false;
    
    // CS 초기 상태를 high로 설정 (active high이므로 초기값은 high)
    driver_do_high(st7920_instance.cs_io);
    
    st7920_driver.cfg = &st7920_instance;
    st7920_driver.opened = true;
    st7920_driver.api = &lcd_api;

    st7920_reset(&st7920_driver);
    
    return &st7920_driver;
}

void st7920_reset(driver_t *drv)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    // CS 초기화 - 비활성화 상태
    driver_do_low(cfg->cs_io);
        
    // 하드웨어 리셋 시퀀스 - DO_LCD_RESET 핀 사용
    driver_do_low(cfg->rst_io);
    st7920_delay_ms(100);
    driver_do_high(cfg->rst_io);
    st7920_delay_ms(50);
    
    // ST7920 초기화 시퀀스 - 참고 라이브러리 기반
    // 1단계: 기본 8비트 기능 설정을 3번 반복 (안정화)
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_ms(5);
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_ms(1);
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_ms(1);
    
    // 2단계: 기본 명령 세트 확정
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_ms(1);
    
    // 3단계: 엔트리 모드 설정 - 커서 자동 증가, 시프트 없음
    st7920_send_cmd(drv, ST7920_CMD_ENTRY_MODE_SET | 0x02);
    st7920_delay_ms(1);
    
    // 4단계: CGRAM 주소 초기화
    st7920_send_cmd(drv, ST7920_CMD_SET_CGRAM_ADDR);
    st7920_delay_ms(1);
    
    // 5단계: DDRAM 주소 초기화
    st7920_send_cmd(drv, ST7920_CMD_SET_DDRAM_ADDR);
    st7920_delay_ms(1);
    
    // 6단계: 디스플레이 제어 - 디스플레이 ON, 커서 OFF, 깜박임 OFF
    st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CONTROL | ST7920_DISPLAY_ON);
    st7920_delay_ms(1);
    
    // 7단계: 화면 지우기
    st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CLEAR); 
    st7920_delay_ms(20);
    
    // 8단계: 홈 위치로 이동
    st7920_send_cmd(drv, ST7920_CMD_RETURN_HOME);
    st7920_delay_ms(2);
    
    

    cfg->initialized = true;
}

void st7920_send_byte(driver_t *drv, uint8_t sync, uint8_t data)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    // ST7920 시리얼 통신 시퀀스 - 참고 라이브러리 기반
    driver_do_high(cfg->cs_io);  // CS HIGH (활성화)
    st7920_delay_us(1);
    
    // 3바이트 시리얼 프로토콜
    driver_spi_send_byte(cfg->spi_io, sync);                    // 동기 바이트 (0xF8 or 0xFA)
    driver_spi_send_byte(cfg->spi_io, data & 0xF0);            // 상위 4비트
    driver_spi_send_byte(cfg->spi_io, (data << 4) & 0xF0);     // 하위 4비트
    
    driver_do_low(cfg->cs_io);   // CS LOW (비활성화)
    st7920_delay_us(100);        // 명령 처리 대기
}

void st7920_send_cmd(driver_t *drv, uint8_t cmd)
{
    st7920_send_byte(drv, ST7920_SYNC_CMD, cmd);

}

void st7920_send_data(driver_t *drv, uint8_t data)
{
    st7920_send_byte(drv, ST7920_SYNC_DATA, data);

}

/**
 * @brief 프레임버퍼의 내용을 LCD 화면 전체에 올바르게 전송합니다.
 */
void st7920_flush_buffer(driver_t *drv)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    if(!cfg->graphic_mode) return;
    
    // 상단 영역 (Y: 0~31)
    for (uint8_t y = 0; y < 32; y++)
    {
        st7920_send_cmd(drv, 0x80 | y);      // Y 주소 설정
        st7920_send_cmd(drv, 0x80);          // X 주소 0으로 설정
        
        // 한 행의 8바이트를 연속 전송 (X 주소 자동 증가)
        for (uint8_t x_byte = 0; x_byte < 16; x_byte++)
        {
            st7920_send_data(drv, framebuffer[y][x_byte]);
        }
    }
    
    // 하단 영역 (Y: 32~63)
    for (uint8_t y = 32; y < 64; y++)
    {
        st7920_send_cmd(drv, 0x80 | (y-32));  // Y 주소 설정
        st7920_send_cmd(drv, 0x88);           // X 주소 8로 설정
        
        // 한 행의 8바이트를 연속 전송
        for (uint8_t x_byte = 0; x_byte < 16; x_byte++)
        {
            st7920_send_data(drv, framebuffer[y][x_byte]);
        }
    }
}
void st7920_clear_screen(driver_t *drv)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    
    if(cfg->graphic_mode)
    {
      
      memset(framebuffer,0,sizeof(framebuffer));
     
      st7920_flush_buffer(drv);
      
    }
    else
    {
    

        // 문자 모드: DISPLAY_CLEAR 명령으로 한번에 지우기
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
    
    if(enable && !cfg->graphic_mode)
    {
        // 그래픽 모드 활성화 - U8g2 기반 시퀀스
        // 1단계: 확장 명령 세트 활성화
        st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT | ST7920_FUNCTION_SET_EXTEND);
        st7920_delay_ms(1);
        
        // 2단계: 그래픽 모드 활성화 (확장 + 그래픽)
        st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT | 
                           ST7920_FUNCTION_SET_EXTEND | ST7920_FUNCTION_SET_GRAPHIC);
        st7920_delay_ms(1);
        
        cfg->graphic_mode = true;
    }
    else if(!enable && cfg->graphic_mode)
    {
        // 문자 모드로 복귀
        // 1단계: 확장 명령 세트만 활성화 (그래픽 비활성화)
        st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT | ST7920_FUNCTION_SET_EXTEND);
        st7920_delay_ms(1);
        
        // 2단계: 기본 명령 세트로 완전 복귀
        st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
        st7920_delay_ms(1);
        
        cfg->graphic_mode = false;
    }
}

void st7920_set_position(driver_t *drv, uint8_t row, uint8_t col)
{
  uint8_t addr;

  if (row == 0)
  {
    addr = 0x80 + col;
  }
  else if (row == 1)
  {
    addr = 0x90 + col;
  }
  else if (row == 2)
  {

    addr = 0x88 + col;
  }
  else if (row == 3)
  {

    addr = 0x98 + col;
  }
  else
  {
    return; // 잘못된 row
  }

  st7920_send_cmd(drv, addr);
}




void st7920_write_string(driver_t *drv, int row, int col, const char *str)
{
    if (!str || row > 3 || col > 15 || row < 0 || col < 0) {
        return;
    }
    
    int current_row = row;
    int current_col = col;
    
    // 첫 문자 위치 설정
    st7920_set_position(drv, current_row, current_col);
    
    while (*str && current_row <= 3) {
        // ST7920의 8+8 문자 분할 주소 매핑을 고려한 문자 출력
        while (*str && current_col < 16) {
            // 8번째 문자 (col=8)에서 주소 점프 발생
            if (current_col == 8) {
                st7920_set_position(drv, current_row, current_col);
            }
            
            st7920_send_data(drv, *str++);
            current_col++;
        }
        
        // 행 끝에 도달하면 다음 행으로 이동
        if (*str && current_col >= 16) {
            current_row++;
            current_col = 0;
            if (current_row <= 3) {
                st7920_set_position(drv, current_row, current_col);
            }
        }
    }
}

// 기존 함수와의 호환성을 위한 래퍼 함수
void st7920_write_string_simple(driver_t *drv, const char *str)
{
    while(*str)
    {
        st7920_send_data(drv, *str++);
    }
}

// 오직 프레임버퍼의 픽셀 값만 변경하는 함수
void st7920_set_pixel(driver_t *drv, uint8_t x, uint8_t y, bool on)
{
    // 좌표 경계 값 체크
    if(x >= ST7920_WIDTH || y >= ST7920_HEIGHT)
        return;
    
    // 수정할 바이트 및 비트 위치 계산
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

static void st7920_set_mode(driver_t *drv, eLCD_MODE_t lcd_mode)
{
    switch(lcd_mode)
    {
        case eLCD_MODE_CHARACTER:
            st7920_set_graphic_mode(drv, false);
            break;
        case eLCD_MODE_GRAPHIC:
            st7920_set_graphic_mode(drv, true);
            break;
        default:
            break;
    }
}

// API 호환성을 위한 래퍼 함수
static void st7920_write_string_api(driver_t *drv, const char *str)
{
    // 현재 커서 위치에서 문자열 출력 (기존 동작 유지)
    st7920_write_string_simple(drv, str);
}