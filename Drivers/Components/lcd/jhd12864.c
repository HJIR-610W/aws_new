/**
 * @file jhd12864.c
 * @brief JHD12864 128x64 그래픽 LCD 드라이버 (패러럴 인터페이스 with CS1/CS2)
 */

#define JHD12864_SPI_USE 0
#define JHD12864_GPIO_USE 0
#define JHD12864_MEM_USE 1

#include "jhd12864.h"
#include <string.h>
#include "driver_stm32_spi.h"
#include "driver_stm32_do.h"
#include "bsp_do.h"
#include "pcb_define.h"
#include "driver_lcd_define.h"

#include "bsp_delay.h"
#include <math.h>
#include <stdlib.h>

#define JHD12864_WIDTH 128
#define JHD12864_HEIGHT 64
#define JHD12864_HALF_WIDTH 64

/* JHD12864 기본 명령어 */
#define JHD12864_CMD_DISPLAY_ON_OFF 0x3E
#define JHD12864_CMD_DISPLAY_START_LINE 0xC0
#define JHD12864_CMD_SET_PAGE 0xB8
#define JHD12864_CMD_SET_COLUMN 0x40
#define JHD12864_CMD_READ_WRITE_MODE 0x3F
#define JHD12864_CMD_RESET 0xE2

/* 디스플레이 제어 비트 */
#define JHD12864_DISPLAY_ON 0x01
#define JHD12864_DISPLAY_OFF 0x00

/* 페이지 및 컬럼 최대값 */
#define JHD12864_MAX_PAGE 7
#define JHD12864_MAX_COLUMN 63  // 각 반쪽당 64 컬럼


#if JHD12864_MEM_USE
// A0=RS, A1=CS1, A2=CS2
// CS1 선택 (좌측): A1=1, A2=0, RS=0/1
#define LCD_CS1_COMMAND_ADDRESS ((uint32_t)(0x60000002))  // A1=1, A2=0, A0=0 
#define LCD_CS1_DATA_ADDRESS    ((uint32_t)(0x60000003))  // A1=1, A2=0, A0=1
// CS2 선택 (우측): A1=0, A2=1, RS=0/1  
#define LCD_CS2_COMMAND_ADDRESS ((uint32_t)(0x60000004))  // A1=0, A2=1, A0=0
#define LCD_CS2_DATA_ADDRESS    ((uint32_t)(0x60000005))  // A1=0, A2=1, A0=1

volatile uint8_t* p_lcd_cs1_cmd  = (volatile uint8_t*)LCD_CS1_COMMAND_ADDRESS;
volatile uint8_t* p_lcd_cs1_data = (volatile uint8_t*)LCD_CS1_DATA_ADDRESS;
volatile uint8_t* p_lcd_cs2_cmd  = (volatile uint8_t*)LCD_CS2_COMMAND_ADDRESS;
volatile uint8_t* p_lcd_cs2_data = (volatile uint8_t*)LCD_CS2_DATA_ADDRESS;
#endif

typedef struct
{
  driver_t *spi_io;
  int rst_do_num;
  bool initialized;
  bool graphic_mode;
  uint8_t current_page;
  uint8_t current_column;
  jhd12864_cs_t current_cs;
} jhd12864_t;

static jhd12864_t jhd12864_instance;
static driver_t jhd12864_driver;
static uint8_t framebuffer[JHD12864_HEIGHT / 8][JHD12864_WIDTH];  // 8 pages x 128 columns

static void jhd12864_set_mode(driver_t *drv, eLCD_MODE_t lcd_mode);
static void jhd12864_write_string_api(driver_t *drv, const char *str);

lcd_api_t jhd12864_lcd_api = {
    .set_position = jhd12864_set_position,
    .write_string_at = jhd12864_write_string,
    .clear_screen = jhd12864_clear_screen,
    .home = jhd12864_home,
    .display_on = jhd12864_display_on,
    .display_off = jhd12864_display_off,
    .set_mode = jhd12864_set_mode,
    .set_pixel = jhd12864_set_pixel,
    .draw_line = jhd12864_draw_line
};

inline void jhd12864_delay_ms(uint32_t ms)
{
  osDelay(ms);
}

inline void jhd12864_delay_us(uint32_t us_delay)
{
  usDelay(us_delay);
}

jhd12864_cs_t jhd12864_get_cs_from_x(uint8_t x)
{
    return (x < JHD12864_HALF_WIDTH) ? JHD12864_CS_LEFT : JHD12864_CS_RIGHT;
}

void jhd12864_select_cs(driver_t *drv, jhd12864_cs_t cs)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    cfg->current_cs = cs;
    // MEM_USE 모드에서는 주소 라인 A1, A2로 CS 선택이 자동으로 됨
}

void jhd12864_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    
    // Configure RS (Register Select) - FSMC_A0 pin as GPIO output
    GPIO_InitStruct.Pin = FSMC_A0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_A0_GPIO_Port, &GPIO_InitStruct);
    
    // Configure RW (Read/Write) - FSMC_NWE pin as GPIO output
    GPIO_InitStruct.Pin = FSMC_NWE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_NWE_GPIO_Port, &GPIO_InitStruct);
    
    // Configure E (Enable) - FSMC_NE1 pin as GPIO output
    GPIO_InitStruct.Pin = FSMC_NE1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_NE1_GPIO_Port, &GPIO_InitStruct);
    
    // Configure 8-bit Data Bus (D0-D7) as GPIO outputs
    // FSMC_D0-D1 on GPIOD (Pins 14-15)
    GPIO_InitStruct.Pin = FSMC_D0_PIN | FSMC_D1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_D0_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D2-D3 on GPIOD (Pins 0-1)
    GPIO_InitStruct.Pin = FSMC_D2 | FSMC_D3_PIN;
    HAL_GPIO_Init(FSMC_D2_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D4-D7 on GPIOE (Pins 7-10)
    GPIO_InitStruct.Pin = FSMC_D4_PIN | FSMC_D5_PIN | FSMC_D6_PIN | FSMC_D7_PIN;
    HAL_GPIO_Init(FSMC_D4_GPIO_Port, &GPIO_InitStruct);
    
    // Initialize control signals to idle state
    HAL_GPIO_WritePin(FSMC_A0_GPIO_Port, FSMC_A0, GPIO_PIN_RESET);        // RS = 0
    HAL_GPIO_WritePin(FSMC_NWE_GPIO_Port, FSMC_NWE_PIN, GPIO_PIN_SET);     // RW = 1 (Read mode)
    HAL_GPIO_WritePin(FSMC_NE1_GPIO_Port, FSMC_NE1_PIN, GPIO_PIN_RESET);   // E = 0 (Disabled)
}

#if JHD12864_GPIO_USE

void jhd12864_gpio_set_data_bus(uint8_t data)
{
    // Set D0 (GPIOD Pin 14)
    HAL_GPIO_WritePin(FSMC_D0_GPIO_Port, FSMC_D0_PIN, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D1 (GPIOD Pin 15)
    HAL_GPIO_WritePin(FSMC_D1_GPIO_Port, FSMC_D1_PIN, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D2 (GPIOD Pin 0)
    HAL_GPIO_WritePin(FSMC_D2_GPIO_Port, FSMC_D2, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D3 (GPIOD Pin 1)
    HAL_GPIO_WritePin(FSMC_D3_GPIO_Port, FSMC_D3_PIN, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D4 (GPIOE Pin 7)
    HAL_GPIO_WritePin(FSMC_D4_GPIO_Port, FSMC_D4_PIN, (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D5 (GPIOE Pin 8)
    HAL_GPIO_WritePin(FSMC_D5_GPIO_Port, FSMC_D5_PIN, (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D6 (GPIOE Pin 9)
    HAL_GPIO_WritePin(FSMC_D6_GPIO_Port, FSMC_D6_PIN, (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D7 (GPIOE Pin 10)
    HAL_GPIO_WritePin(FSMC_D7_GPIO_Port, FSMC_D7_PIN, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void jhd12864_gpio_set_data_bus_input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Configure 8-bit Data Bus (D0-D7) as inputs for reading
    // FSMC_D0-D1 on GPIOD (Pins 14-15)
    GPIO_InitStruct.Pin = FSMC_D0_PIN | FSMC_D1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(FSMC_D0_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D2-D3 on GPIOD (Pins 0-1)
    GPIO_InitStruct.Pin = FSMC_D2 | FSMC_D3_PIN;
    HAL_GPIO_Init(FSMC_D2_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D4-D7 on GPIOE (Pins 7-10)
    GPIO_InitStruct.Pin = FSMC_D4_PIN | FSMC_D5_PIN | FSMC_D6_PIN | FSMC_D7_PIN;
    HAL_GPIO_Init(FSMC_D4_GPIO_Port, &GPIO_InitStruct);
}

void jhd12864_gpio_set_data_bus_output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Configure 8-bit Data Bus (D0-D7) as outputs for writing
    // FSMC_D0-D1 on GPIOD (Pins 14-15)
    GPIO_InitStruct.Pin = FSMC_D0_PIN | FSMC_D1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_D0_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D2-D3 on GPIOD (Pins 0-1)
    GPIO_InitStruct.Pin = FSMC_D2 | FSMC_D3_PIN;
    HAL_GPIO_Init(FSMC_D2_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D4-D7 on GPIOE (Pins 7-10)
    GPIO_InitStruct.Pin = FSMC_D4_PIN | FSMC_D5_PIN | FSMC_D6_PIN | FSMC_D7_PIN;
    HAL_GPIO_Init(FSMC_D4_GPIO_Port, &GPIO_InitStruct);
}

uint8_t jhd12864_gpio_read_data_bus(void)
{
    uint8_t data = 0;
    
    // Read D0 (GPIOD Pin 14)
    if (HAL_GPIO_ReadPin(FSMC_D0_GPIO_Port, FSMC_D0_PIN) == GPIO_PIN_SET)
        data |= 0x01;
    
    // Read D1 (GPIOD Pin 15)
    if (HAL_GPIO_ReadPin(FSMC_D1_GPIO_Port, FSMC_D1_PIN) == GPIO_PIN_SET)
        data |= 0x02;
    
    // Read D2 (GPIOD Pin 0)
    if (HAL_GPIO_ReadPin(FSMC_D2_GPIO_Port, FSMC_D2) == GPIO_PIN_SET)
        data |= 0x04;
    
    // Read D3 (GPIOD Pin 1)
    if (HAL_GPIO_ReadPin(FSMC_D3_GPIO_Port, FSMC_D3_PIN) == GPIO_PIN_SET)
        data |= 0x08;
    
    // Read D4 (GPIOE Pin 7)
    if (HAL_GPIO_ReadPin(FSMC_D4_GPIO_Port, FSMC_D4_PIN) == GPIO_PIN_SET)
        data |= 0x10;
    
    // Read D5 (GPIOE Pin 8)
    if (HAL_GPIO_ReadPin(FSMC_D5_GPIO_Port, FSMC_D5_PIN) == GPIO_PIN_SET)
        data |= 0x20;
    
    // Read D6 (GPIOE Pin 9)
    if (HAL_GPIO_ReadPin(FSMC_D6_GPIO_Port, FSMC_D6_PIN) == GPIO_PIN_SET)
        data |= 0x40;
    
    // Read D7 (GPIOE Pin 10)
    if (HAL_GPIO_ReadPin(FSMC_D7_GPIO_Port, FSMC_D7_PIN) == GPIO_PIN_SET)
        data |= 0x80;
    
    return data;
}

#define JHD12864_E_HIGH()     HAL_GPIO_WritePin(FSMC_NE1_GPIO_Port, FSMC_NE1_PIN, GPIO_PIN_SET);
#define JHD12864_E_LOW()      HAL_GPIO_WritePin(FSMC_NE1_GPIO_Port, FSMC_NE1_PIN, GPIO_PIN_RESET);

#define JHD12864_CMD_SET()    HAL_GPIO_WritePin(FSMC_A0_GPIO_Port, FSMC_A0, GPIO_PIN_RESET);  // RS = 0 for command
#define JHD12864_DATA_SET()   HAL_GPIO_WritePin(FSMC_A0_GPIO_Port, FSMC_A0, GPIO_PIN_SET);    // RS = 1 for data

#define JHD12864_W_SET()      HAL_GPIO_WritePin(FSMC_NWE_GPIO_Port, FSMC_NWE_PIN, GPIO_PIN_RESET); // RW = 0 for write
#define JHD12864_R_SET()      HAL_GPIO_WritePin(FSMC_NWE_GPIO_Port, FSMC_NWE_PIN, GPIO_PIN_SET);   // RW = 1 for read

void jhd12864_gpio_write_byte(uint8_t data, bool is_cmd)
{
    // Set RS (Register Select) pin
    if (is_cmd) {
        JHD12864_CMD_SET();
    } else {
        JHD12864_DATA_SET();
    }
    
    JHD12864_W_SET();
    
    // Set data on data bus
    jhd12864_gpio_set_data_bus(data);
    
    // Enable pulse
    JHD12864_E_HIGH();
    jhd12864_delay_us(1);
    JHD12864_E_LOW();
    jhd12864_delay_us(1);
}

uint8_t jhd12864_gpio_read_byte(bool is_cmd)
{
    uint8_t data = 0;
    
    // Set data bus to input mode
    jhd12864_gpio_set_data_bus_input();
    
    // Set RS (Register Select) pin
    if (is_cmd) {
        JHD12864_CMD_SET();  // RS = 0 for status/command register
    } else {
        JHD12864_DATA_SET(); // RS = 1 for data register
    }
    
    // Set RW = 1 for read operation
    JHD12864_R_SET();
    
    // Setup time
    jhd12864_delay_us(1);
    
    // Enable pulse (E high)
    JHD12864_E_HIGH();
    
    // Enable pulse width
    jhd12864_delay_us(1);
    
    // Read data from data bus
    data = jhd12864_gpio_read_data_bus();
    
    // Enable pulse (E low)
    JHD12864_E_LOW();
    
    // Restore data bus to output mode
    jhd12864_gpio_set_data_bus_output();
    
    // Hold time
    jhd12864_delay_us(1);
    
    return data;
}
#endif

uint8_t jhd12864_read_status(driver_t *drv)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    uint8_t status = 0;
    
#if JHD12864_SPI_USE
    return 0x00;
#endif

#if JHD12864_GPIO_USE
    status = jhd12864_gpio_read_byte(true);
#endif

#if JHD12864_MEM_USE
    switch(cfg->current_cs) {
        case JHD12864_CS_LEFT:
            status = *p_lcd_cs1_cmd;
            break;
        case JHD12864_CS_RIGHT:
            status = *p_lcd_cs2_cmd;
            break;
        case JHD12864_CS_BOTH:
            // 양쪽 모두 선택시 좌측 상태 반환
            status = *p_lcd_cs1_cmd;
            break;
    }
#endif

    return status;
}

uint8_t jhd12864_read_data(driver_t *drv)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    uint8_t data = 0;
    
#if JHD12864_SPI_USE
    return 0x00;
#endif

#if JHD12864_GPIO_USE
    data = jhd12864_gpio_read_byte(false);
#endif

#if JHD12864_MEM_USE
    switch(cfg->current_cs) {
        case JHD12864_CS_LEFT:
            data = *p_lcd_cs1_data;
            break;
        case JHD12864_CS_RIGHT:
            data = *p_lcd_cs2_data;
            break;
        case JHD12864_CS_BOTH:
            // 양쪽 모두 선택시 좌측 데이터 반환
            data = *p_lcd_cs1_data;
            break;
    }
#endif

    return data;
}

bool jhd12864_is_busy(driver_t *drv)
{
    uint8_t status = jhd12864_read_status(drv);
    return (status & 0x80) != 0;  // Bit 7 is busy flag
}

void jhd12864_wait_ready(driver_t *drv)
{
    uint32_t timeout = 10000;  // 10ms timeout
    
    while (jhd12864_is_busy(drv) && timeout > 0) {
        jhd12864_delay_us(1);
        timeout--;
    }
}

driver_t *jhd12864_open(void)
{
    if(jhd12864_driver.opened)
    {
        return &jhd12864_driver;
    }

#if JHD12864_SPI_USE
    jhd12864_instance.spi_io = driver_spi_open(STM_SPI_1);
    if(!jhd12864_instance.spi_io)
    {
        return NULL;
    }

    jhd12864_instance.cs_io = driver_do_open(DO_LCD_CS, NULL);
    if(!jhd12864_instance.cs_io)
    {
        return NULL;
    }
    
    bsp_do_high(jhd12864_instance.cs_io);
#endif

#if JHD12864_GPIO_USE
    // Initialize GPIO pins for FSMC interface
    jhd12864_gpio_init();
#endif

    jhd12864_instance.rst_do_num = BSP_DO_LCD_RESET;

    jhd12864_instance.initialized = false;
    jhd12864_instance.graphic_mode = true;  // JHD12864 is primarily a graphic LCD
    jhd12864_instance.current_page = 0;
    jhd12864_instance.current_column = 0;
    jhd12864_instance.current_cs = JHD12864_CS_LEFT;
    
    jhd12864_driver.cfg = &jhd12864_instance;
    jhd12864_driver.opened = true;
    jhd12864_driver.api = &jhd12864_lcd_api;

    jhd12864_reset(&jhd12864_driver);
    
    return &jhd12864_driver;
}

void jhd12864_reset(driver_t *drv)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    
    // 하드웨어 리셋 시퀀스
    bsp_do_low(cfg->rst_do_num);
    jhd12864_delay_ms(100);
    bsp_do_high(cfg->rst_do_num);
    jhd12864_delay_ms(50);
    
    // JHD12864 초기화 시퀀스 - 양쪽 CS 모두 초기화
    jhd12864_select_cs(drv, JHD12864_CS_BOTH);
    
    // 1. 시스템 리셋
    jhd12864_send_cmd(drv, JHD12864_CMD_RESET);
    jhd12864_delay_ms(10);
    
    // 2. 디스플레이 ON
    jhd12864_send_cmd(drv, JHD12864_CMD_DISPLAY_ON_OFF | JHD12864_DISPLAY_ON);
    jhd12864_delay_ms(1);
    
    // 3. 시작 라인 설정 (0)
    jhd12864_send_cmd(drv, JHD12864_CMD_DISPLAY_START_LINE | 0x00);
    jhd12864_delay_ms(1);
    
    // 좌측 반쪽 초기화
    jhd12864_select_cs(drv, JHD12864_CS_LEFT);
    jhd12864_send_cmd(drv, JHD12864_CMD_SET_PAGE | 0x00);
    jhd12864_send_cmd(drv, JHD12864_CMD_SET_COLUMN | 0x00);
    jhd12864_delay_ms(1);
    
    // 우측 반쪽 초기화  
    jhd12864_select_cs(drv, JHD12864_CS_RIGHT);
    jhd12864_send_cmd(drv, JHD12864_CMD_SET_PAGE | 0x00);
    jhd12864_send_cmd(drv, JHD12864_CMD_SET_COLUMN | 0x00);
    jhd12864_delay_ms(1);
    
    // 화면 지우기
    jhd12864_clear_screen(drv);
    
    cfg->initialized = true;
}

void jhd12864_send_cmd(driver_t *drv, uint8_t cmd)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    
#if JHD12864_SPI_USE 
    // SPI mode not implemented for JHD12864
#endif
    
#if JHD12864_GPIO_USE
    jhd12864_gpio_write_byte(cmd, true);
    jhd12864_delay_us(72);
#endif

#if JHD12864_MEM_USE
    switch(cfg->current_cs) {
        case JHD12864_CS_LEFT:
            *p_lcd_cs1_cmd = cmd;
            break;
        case JHD12864_CS_RIGHT:
            *p_lcd_cs2_cmd = cmd;
            break;
        case JHD12864_CS_BOTH:
            *p_lcd_cs1_cmd = cmd;
            *p_lcd_cs2_cmd = cmd;
            break;
    }
    jhd12864_delay_us(72);
#endif
}

void jhd12864_send_data(driver_t *drv, uint8_t data)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    
#if JHD12864_SPI_USE 
    // SPI mode not implemented for JHD12864
#endif
    
#if JHD12864_GPIO_USE
    jhd12864_gpio_write_byte(data, false);
    jhd12864_delay_us(72);
#endif
    
#if JHD12864_MEM_USE
    switch(cfg->current_cs) {
        case JHD12864_CS_LEFT:
            *p_lcd_cs1_data = data;
            break;
        case JHD12864_CS_RIGHT:
            *p_lcd_cs2_data = data;
            break;
        case JHD12864_CS_BOTH:
            *p_lcd_cs1_data = data;
            *p_lcd_cs2_data = data;
            break;
    }
    jhd12864_delay_us(72);
#endif
}

void jhd12864_set_page(driver_t *drv, uint8_t page)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    
    if(page > JHD12864_MAX_PAGE)
        return;
    
    jhd12864_send_cmd(drv, JHD12864_CMD_SET_PAGE | page);
    cfg->current_page = page;
}

void jhd12864_set_column(driver_t *drv, uint8_t column)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    
    if(column > JHD12864_MAX_COLUMN)
        return;
    
    jhd12864_send_cmd(drv, JHD12864_CMD_SET_COLUMN | column);
    cfg->current_column = column;
}

void jhd12864_flush_buffer(driver_t *drv)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    
    if(!cfg->graphic_mode) return;
    
    // 좌측 반쪽 (0-63 컬럼) 전송
    jhd12864_select_cs(drv, JHD12864_CS_LEFT);
    for (uint8_t page = 0; page < 8; page++)
    {
        jhd12864_set_page(drv, page);
        jhd12864_set_column(drv, 0);
        
        for (uint8_t col = 0; col < JHD12864_HALF_WIDTH; col++)
        {
            jhd12864_send_data(drv, framebuffer[page][col]);
        }
    }
    
    // 우측 반쪽 (64-127 컬럼) 전송
    jhd12864_select_cs(drv, JHD12864_CS_RIGHT);
    for (uint8_t page = 0; page < 8; page++)
    {
        jhd12864_set_page(drv, page);
        jhd12864_set_column(drv, 0);
        
        for (uint8_t col = JHD12864_HALF_WIDTH; col < JHD12864_WIDTH; col++)
        {
            jhd12864_send_data(drv, framebuffer[page][col]);
        }
    }
}

void jhd12864_clear_screen(driver_t *drv)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    
    if(cfg->graphic_mode)
    {
        // 프레임버퍼 클리어
        memset(framebuffer, 0, sizeof(framebuffer));
        
        // 화면에 반영
        jhd12864_flush_buffer(drv);
    }
    else
    {
        // 좌측 반쪽 클리어
        jhd12864_select_cs(drv, JHD12864_CS_LEFT);
        for (uint8_t page = 0; page < 8; page++)
        {
            jhd12864_set_page(drv, page);
            jhd12864_set_column(drv, 0);
            
            for (uint8_t col = 0; col < JHD12864_HALF_WIDTH; col++)
            {
                jhd12864_send_data(drv, 0x00);
            }
        }
        
        // 우측 반쪽 클리어
        jhd12864_select_cs(drv, JHD12864_CS_RIGHT);
        for (uint8_t page = 0; page < 8; page++)
        {
            jhd12864_set_page(drv, page);
            jhd12864_set_column(drv, 0);
            
            for (uint8_t col = 0; col < JHD12864_HALF_WIDTH; col++)
            {
                jhd12864_send_data(drv, 0x00);
            }
        }
    }
}

void jhd12864_home(driver_t *drv)
{
    jhd12864_set_page(drv, 0);
    jhd12864_set_column(drv, 0);
}

void jhd12864_display_on(driver_t *drv)
{
    jhd12864_send_cmd(drv, JHD12864_CMD_DISPLAY_ON_OFF | JHD12864_DISPLAY_ON);
    jhd12864_delay_us(100);
}

void jhd12864_display_off(driver_t *drv)
{
    jhd12864_send_cmd(drv, JHD12864_CMD_DISPLAY_ON_OFF | JHD12864_DISPLAY_OFF);
    jhd12864_delay_us(100);
}

void jhd12864_set_graphic_mode(driver_t *drv, bool enable)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    
    // JHD12864는 기본적으로 그래픽 LCD이므로 모드 변경이 필요없음
    cfg->graphic_mode = enable;
}

void jhd12864_set_position(driver_t *drv, uint8_t row, uint8_t col)
{
    // JHD12864는 페이지 기반이므로 텍스트 위치 설정
    // 각 페이지는 8픽셀 높이, 각 문자는 8픽셀 너비로 가정
    uint8_t page = row;
    uint8_t column = col * 8;  // 문자 너비 8픽셀
    
    if(page > JHD12864_MAX_PAGE || column > JHD12864_MAX_COLUMN)
        return;
    
    jhd12864_set_page(drv, page);
    jhd12864_set_column(drv, column);
}

void jhd12864_write_string(driver_t *drv, int row, int col, const char *str)
{
    if (!str || row > 7 || col > 15 || row < 0 || col < 0) {
        return;
    }
    
    jhd12864_set_position(drv, row, col);
    
    // 간단한 문자 출력 (실제 구현에서는 폰트 데이터 필요)
    while (*str) {
        // 여기서는 간단히 ASCII 값을 출력
        jhd12864_send_data(drv, *str);
        str++;
    }
}

void jhd12864_write_string_simple(driver_t *drv, const char *str)
{
    while(*str)
    {
        jhd12864_send_data(drv, *str++);
    }
}

void jhd12864_set_pixel(driver_t *drv, uint8_t x, uint8_t y, bool on)
{
    // 좌표 경계 값 체크
    if(x >= JHD12864_WIDTH || y >= JHD12864_HEIGHT)
        return;
    
    // 페이지 및 비트 위치 계산
    uint8_t page = y / 8;
    uint8_t bit_pos = y % 8;
    
    // X 좌표에 따라 CS 선택
    jhd12864_cs_t cs = jhd12864_get_cs_from_x(x);
    jhd12864_select_cs(drv, cs);
    
    // 프레임버퍼 업데이트
    if(on)
    {
        framebuffer[page][x] |= (1 << bit_pos);
    }
    else
    {
        framebuffer[page][x] &= ~(1 << bit_pos);
    }
}

void jhd12864_draw_line(driver_t *drv, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool on)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int x = x1, y = y1;
    
    while(1)
    {
        jhd12864_set_pixel(drv, x, y, on);
        
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

void jhd12864_draw_rect(driver_t *drv, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool fill, bool on)
{
    if(fill)
    {
        for(uint8_t i = 0; i < height; i++)
        {
            for(uint8_t j = 0; j < width; j++)
            {
                jhd12864_set_pixel(drv, x + j, y + i, on);
            }
        }
    }
    else
    {
        jhd12864_draw_line(drv, x, y, x + width - 1, y, on);
        jhd12864_draw_line(drv, x + width - 1, y, x + width - 1, y + height - 1, on);
        jhd12864_draw_line(drv, x + width - 1, y + height - 1, x, y + height - 1, on);
        jhd12864_draw_line(drv, x, y + height - 1, x, y, on);
    }
}

void jhd12864_draw_bitmap(driver_t *drv, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap)
{
    jhd12864_t *cfg = (jhd12864_t *)drv->cfg;
    
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
                    jhd12864_set_pixel(drv, x + col + bit, y + row, true);
                }
            }
        }
    }
}

static void jhd12864_set_mode(driver_t *drv, eLCD_MODE_t lcd_mode)
{
    switch(lcd_mode)
    {
        case eLCD_MODE_CHARACTER:
            jhd12864_set_graphic_mode(drv, false);
            break;
        case eLCD_MODE_GRAPHIC:
            jhd12864_set_graphic_mode(drv, true);
            break;
        default:
            break;
    }
}

static void jhd12864_write_string_api(driver_t *drv, const char *str)
{
    jhd12864_write_string_simple(drv, str);
}