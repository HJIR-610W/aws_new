
#define ST7920_SPI_USE 0
#define ST7920_GPIO_USE 0
#define ST7920_MEM_USE 1

#include "st7920.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "cmsis_os2.h"
#include "bsp_spi.h"
#include "bsp_do.h"
#include "driver_lcd_define.h"
#include "bsp.h"
#include "bsp_delay.h"
#include "drv_power.h"
#include "font\font_6x8.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pcb_define.h"
#include "user_heap.h"

#define ST7920_WIDTH 128
#define ST7920_HEIGHT 64

/* ST7920 기본 명령어 */
#define ST7920_CMD_DISPLAY_CLEAR   0x01
#define ST7920_CMD_RETURN_HOME     0x02
#define ST7920_CMD_ENTRY_MODE_SET  0x04
#define ST7920_CMD_DISPLAY_CONTROL 0x08
#define ST7920_CMD_CURSOR_SHIFT    0x10
#define ST7920_CMD_FUNCTION_SET    0x20
#define ST7920_CMD_SET_CGRAM_ADDR  0x40
#define ST7920_CMD_SET_DDRAM_ADDR  0x80


#define ST7920_DISPLAY_BLINK_ON   0x01
#define ST7920_DISPLAY_CURSOR_ON  0x02
#define ST7920_DISPLAY_DISPLAY_ON 0x04

#define ST7920_FUNCTION_SET_8BIT    0x10
#define ST7920_FUNCTION_SET_EXTEND  0x04   // 확장 명령 세트 활성화 RE 1 확장 명렁어 0 basic


#define ST7920_FUNCTION_SET_GRAPHIC 0x02  // 그래픽 모드 활성화


//확장 명령어
#define ST7920_CMD_SET_SR 0x02






#define ST7920_SYNC_CMD  0xF8   // 명령 전송시 첫 바이트
#define ST7920_SYNC_DATA 0xFA  // 데이터 전송시 첫 바이트


#if ST7920_MEM_USE
#define LCD_COMMAND_ADDRESS ((uint32_t)(0x60000000)) 
#define LCD_DATA_ADDRESS    ((uint32_t)(0x60000001)) 

volatile uint8_t* p_lcd_cmd  = (volatile uint8_t*)LCD_COMMAND_ADDRESS;
volatile uint8_t* p_lcd_data = (volatile uint8_t*)LCD_DATA_ADDRESS;
#endif

typedef struct
{
  int spi_num;
  int cs_do_num;  // CS는 active high
  int rst_do_num;
  bool initialized;
  bool graphic_mode;
} st7920_t;



static st7920_t st7920_instance;
static driver_t st7920_driver;
static uint8_t framebuffer[ST7920_HEIGHT][ST7920_WIDTH / 8];  // 그래픽 모드용 프레임버퍼

static void st7920_set_mode(driver_t *drv, eLCD_MODE_t lcd_mode);

void st7920_flush_buffer(driver_t *drv);
void st7920_put_ch(driver_t *drv, int row, int col, uint8_t ch);

lcd_api_t lcd_api = {.set_position = st7920_set_position,
                     .write_string_at = st7920_write_string,
                     .clear_screen = st7920_clear_screen,
                     .home = st7920_home,
                     .display_on = st7920_display_on,
                     .display_off = st7920_display_off,
                     .set_mode = st7920_set_mode,
                     .set_pixel = st7920_set_pixel,
                     .flush = st7920_flush_buffer,
                     .put_ch = st7920_put_ch};

inline void st7920_delay_ms(uint32_t ms)
{

  osDelay(ms);
  
}

inline void st7920_delay_us(uint32_t us_delay)
{
  bsp_us_delay(us_delay);
}

void st7920_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    
    // Configure RS (Register Select) - FSMC_A0 pin as GPIO output
    GPIO_InitStruct.Pin = FSMC_A0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_A0_GPIO_Port, &GPIO_InitStruct);
    
    // Configure RW (Read/Write) - FSMC_NWE pin as GPIO output
    GPIO_InitStruct.Pin = FSMC_NWE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_NWE_GPIO_Port, &GPIO_InitStruct);
    
    // Configure E (Enable) - FSMC_NE1 pin as GPIO output
    GPIO_InitStruct.Pin = FSMC_NE1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_NE1_GPIO_Port, &GPIO_InitStruct);
    
    // Configure 8-bit Data Bus (D0-D7) as GPIO outputs
    // FSMC_D0-D1 on GPIOD (Pins 14-15)
    GPIO_InitStruct.Pin = FSMC_D0_Pin | FSMC_D1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_D0_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D2-D3 on GPIOD (Pins 0-1)
    GPIO_InitStruct.Pin = FSMC_D2_Pin | FSMC_D3_Pin;
    HAL_GPIO_Init(FSMC_D2_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D4-D7 on GPIOE (Pins 7-10)
    GPIO_InitStruct.Pin = FSMC_D4_Pin | FSMC_D5_Pin | FSMC_D6_Pin | FSMC_D7_Pin;
    HAL_GPIO_Init(FSMC_D4_GPIO_Port, &GPIO_InitStruct);
    
    // Initialize control signals to idle state
    HAL_GPIO_WritePin(FSMC_A0_GPIO_Port, FSMC_A0_Pin, GPIO_PIN_RESET);        // RS = 0
    HAL_GPIO_WritePin(FSMC_NWE_GPIO_Port, FSMC_NWE_Pin, GPIO_PIN_SET);     // RW = 1 (Read mode)
    HAL_GPIO_WritePin(FSMC_NE1_GPIO_Port, FSMC_NE1_Pin, GPIO_PIN_RESET);   // E = 0 (Disabled)
}

#if ST7920_GPIO_USE




void st7920_gpio_set_data_bus(uint8_t data)
{
    // Set D0 (GPIOD Pin 14)
    HAL_GPIO_WritePin(FSMC_D0_GPIO_Port, FSMC_D0_Pin, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D1 (GPIOD Pin 15)
    HAL_GPIO_WritePin(FSMC_D1_GPIO_Port, FSMC_D1_Pin, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D2 (GPIOD Pin 0)
    HAL_GPIO_WritePin(FSMC_D2_GPIO_Port, FSMC_D2_Pin, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D3 (GPIOD Pin 1)
    HAL_GPIO_WritePin(FSMC_D3_GPIO_Port, FSMC_D3_Pin, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D4 (GPIOE Pin 7)
    HAL_GPIO_WritePin(FSMC_D4_GPIO_Port, FSMC_D4_Pin, (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D5 (GPIOE Pin 8)
    HAL_GPIO_WritePin(FSMC_D5_GPIO_Port, FSMC_D5_Pin, (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D6 (GPIOE Pin 9)
    HAL_GPIO_WritePin(FSMC_D6_GPIO_Port, FSMC_D6_Pin, (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Set D7 (GPIOE Pin 10)
    HAL_GPIO_WritePin(FSMC_D7_GPIO_Port, FSMC_D7_Pin, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void st7920_gpio_set_data_bus_input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Configure 8-bit Data Bus (D0-D7) as inputs for reading
    // FSMC_D0-D1 on GPIOD (Pins 14-15)
    GPIO_InitStruct.Pin = FSMC_D0_Pin | FSMC_D1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(FSMC_D0_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D2-D3 on GPIOD (Pins 0-1)
    GPIO_InitStruct.Pin = FSMC_D2_Pin | FSMC_D3_Pin;
    HAL_GPIO_Init(FSMC_D2_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D4-D7 on GPIOE (Pins 7-10)
    GPIO_InitStruct.Pin = FSMC_D4_Pin | FSMC_D5_Pin | FSMC_D6_Pin | FSMC_D7_Pin;
    HAL_GPIO_Init(FSMC_D4_GPIO_Port, &GPIO_InitStruct);
}

void st7920_gpio_set_data_bus_output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Configure 8-bit Data Bus (D0-D7) as outputs for writing
    // FSMC_D0-D1 on GPIOD (Pins 14-15)
    GPIO_InitStruct.Pin = FSMC_D0_Pin | FSMC_D1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FSMC_D0_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D2-D3 on GPIOD (Pins 0-1)
    GPIO_InitStruct.Pin = FSMC_D2_Pin | FSMC_D3_Pin;
    HAL_GPIO_Init(FSMC_D2_GPIO_Port, &GPIO_InitStruct);
    
    // FSMC_D4-D7 on GPIOE (Pins 7-10)
    GPIO_InitStruct.Pin = FSMC_D4_Pin | FSMC_D5_Pin | FSMC_D6_Pin | FSMC_D7_Pin;
    HAL_GPIO_Init(FSMC_D4_GPIO_Port, &GPIO_InitStruct);
}

uint8_t st7920_gpio_read_data_bus(void)
{
    uint8_t data = 0;
    
    // Read D0 (GPIOD Pin 14)
    if (HAL_GPIO_ReadPin(FSMC_D0_GPIO_Port, FSMC_D0_Pin) == GPIO_PIN_SET)
        data |= 0x01;
    
    // Read D1 (GPIOD Pin 15)
    if (HAL_GPIO_ReadPin(FSMC_D1_GPIO_Port, FSMC_D1_Pin) == GPIO_PIN_SET)
        data |= 0x02;
    
    // Read D2 (GPIOD Pin 0)
    if (HAL_GPIO_ReadPin(FSMC_D2_GPIO_Port, FSMC_D2_Pin) == GPIO_PIN_SET)
        data |= 0x04;
    
    // Read D3 (GPIOD Pin 1)
    if (HAL_GPIO_ReadPin(FSMC_D3_GPIO_Port, FSMC_D3_Pin) == GPIO_PIN_SET)
        data |= 0x08;
    
    // Read D4 (GPIOE Pin 7)
    if (HAL_GPIO_ReadPin(FSMC_D4_GPIO_Port, FSMC_D4_Pin) == GPIO_PIN_SET)
        data |= 0x10;
    
    // Read D5 (GPIOE Pin 8)
    if (HAL_GPIO_ReadPin(FSMC_D5_GPIO_Port, FSMC_D5_Pin) == GPIO_PIN_SET)
        data |= 0x20;
    
    // Read D6 (GPIOE Pin 9)
    if (HAL_GPIO_ReadPin(FSMC_D6_GPIO_Port, FSMC_D6_Pin) == GPIO_PIN_SET)
        data |= 0x40;
    
    // Read D7 (GPIOE Pin 10)
    if (HAL_GPIO_ReadPin(FSMC_D7_GPIO_Port, FSMC_D7_Pin) == GPIO_PIN_SET)
        data |= 0x80;
    
    return data;
}

#define ST7920_E_HIGH()     HAL_GPIO_WritePin(FSMC_NE1_GPIO_Port, FSMC_NE1_Pin,GPIO_PIN_RESET  );

#define ST7920_E_LOW()     HAL_GPIO_WritePin(FSMC_NE1_GPIO_Port, FSMC_NE1_Pin, GPIO_PIN_SET); 

#define ST7920_CMD_SET()  HAL_GPIO_WritePin(FSMC_A0_GPIO_Port, FSMC_A0_Pin, GPIO_PIN_RESET);  // RS = 0 for command
#define ST7920_DATA_SET()   HAL_GPIO_WritePin(FSMC_A0_GPIO_Port, FSMC_A0_Pin, GPIO_PIN_SET);    // RS = 1 for data

#define ST7920_W_SET()     HAL_GPIO_WritePin(FSMC_NWE_GPIO_Port, FSMC_NWE_Pin, GPIO_PIN_RESET); // Set RW = 0 for write operation
#define ST7920_R_SET()      HAL_GPIO_WritePin(FSMC_NWE_GPIO_Port, FSMC_NWE_Pin, GPIO_PIN_SET); // Set RW = 0 for write operation

void st7920_gpio_write_byte(uint8_t data, bool is_cmd)
{

    // Set RS (Register Select) pin
    if (is_cmd) {
      ST7920_CMD_SET();
       
    } else {
      ST7920_DATA_SET();
    }
         ST7920_W_SET();

    // Set data on data bus
    st7920_gpio_set_data_bus(data);

    // Small setup time
   // st7920_delay_us(1);
        ST7920_E_HIGH();

    ST7920_E_LOW();

  //  st7920_delay_us(1);


    

  //  st7920_delay_us(1);

}

    
 
uint8_t st7920_gpio_read_byte(bool is_cmd)
{
    uint8_t data = 0;
    
    // Set data bus to input mode
    st7920_gpio_set_data_bus_input();
    
    // Set RS (Register Select) pin
    if (is_cmd) {
        ST7920_CMD_SET();  // RS = 0 for status/command register
    } else {
        ST7920_DATA_SET(); // RS = 1 for data register
    }
    
    // Set RW = 1 for read operation
    ST7920_R_SET();
    
    // Setup time
    st7920_delay_us(1);
    
    // Enable pulse (E high)
    ST7920_E_HIGH();
    
    // Enable pulse width (minimum 450ns for ST7920)
    st7920_delay_us(1);
    
    // Read data from data bus
    data = st7920_gpio_read_data_bus();
    
    // Enable pulse (E low)
    ST7920_E_LOW();
    
    // Restore data bus to output mode
    st7920_gpio_set_data_bus_output();
    
    // Hold time
    st7920_delay_us(1);
    
    return data;
}
#endif

uint8_t st7920_read_status(driver_t *drv)
{
    uint8_t status = 0;
    
#if ST7920_SPI_USE
    // SPI mode doesn't support status reading in standard implementation
    return 0x00;
#endif

#if ST7920_GPIO_USE
    status = st7920_gpio_read_byte(true);  // true = read status register
#endif

#if ST7920_MEM_USE
    status = *p_lcd_cmd;  // Read status register via memory mapping
#endif

    return status;
}

uint8_t st7920_read_data(driver_t *drv)
{
    uint8_t data = 0;
    
#if ST7920_SPI_USE
    // SPI mode doesn't support data reading in standard implementation
    return 0x00;
#endif

#if ST7920_GPIO_USE
    data = st7920_gpio_read_byte(false);  // false = read data register
#endif

#if ST7920_MEM_USE
    data = *p_lcd_data;  // Read data register via memory mapping
#endif

    return data;
}

bool st7920_is_busy(driver_t *drv)
{
    uint8_t status = st7920_read_status(drv);
    return (status & 0x80) != 0;  // Bit 7 is busy flag
}

void st7920_wait_ready(driver_t *drv)
{
    uint32_t timeout = 10000;  // 10ms timeout
    
    while (st7920_is_busy(drv) && timeout > 0) {
        st7920_delay_us(1);
        timeout--;
    }
}



driver_t *st7920_open(void)
{
    if(st7920_driver.opened)
    {
        return &st7920_driver;
    }

    drv_power_on(DRV_POWER_LCD);
    
#if ST7920_SPI_USE
    st7920_instance.spi_num = BSP_SPI_1;
    if(!st7920_instance.spi_num)
    {
        return NULL;
    }

    st7920_instance.cs_do_num = DO_LCD_CS;
    if(!st7920_instance.cs_do_num)
    {
        return NULL;
    }

    bsp_spi_init(st7920_instance.spi_num);
     bsp_do_high(st7920_instance.cs_do_num);
#endif

#if ST7920_GPIO_USE
    // Initialize GPIO pins for FSMC interface
    st7920_gpio_init();
#endif

    st7920_instance.rst_do_num = BSP_DO_LCD_RESET;

    st7920_instance.initialized = false;
    st7920_instance.graphic_mode = false;
    
    st7920_driver.cfg = &st7920_instance;
    st7920_driver.opened = true;
    st7920_driver.api = &lcd_api;

    st7920_reset(&st7920_driver);

   // st7920_set_mode(&st7920_driver,eLCD_MODE_GRAPHIC);
    
    return &st7920_driver;
}

void st7920_close(void)
{
  st7920_driver.opened = NULL;
  drv_power_off(DRV_POWER_LCD);
}


void st7920_send_cmd(driver_t *drv, uint8_t cmd)
{
#if ST7920_SPI_USE 
    st7920_send_byte(drv, ST7920_SYNC_CMD, cmd);
#endif
    
#if ST7920_GPIO_USE
    st7920_gpio_write_byte(cmd, true);  // true = command mode
    st7920_delay_us(72);  // ST7920 command execution time
#endif

#if ST7920_MEM_USE

    *p_lcd_cmd = cmd;

    st7920_delay_us(72);  // ST7920 command execution time (typical)
#endif
}

void st7920_reset(driver_t *drv)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    // CS 초기화 - 비활성화 상태
#if ST7920_SPI_USE
    bsp_do_low(cfg->cs_do_num);
#endif
    // 하드웨어 리셋 시퀀스 - DO_LCD_RESET 핀 사용

    st7920_delay_ms(40);    
    bsp_do_low(cfg->rst_do_num);
    st7920_delay_ms(1);
    bsp_do_high(cfg->rst_do_num);
 

    // 3번 반복 안정화
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_ms(1);
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_ms(1);
    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT);
    st7920_delay_ms(1);

   
    st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CONTROL );
    st7920_delay_ms(1);
    
    st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CLEAR);
    st7920_delay_ms(1);

          
    st7920_send_cmd(drv, ST7920_CMD_ENTRY_MODE_SET | 0x02); // 엔트리 모드 설정 - 커서 자동 증가, 시프트 없음
    st7920_delay_ms(1);
    
    st7920_send_cmd(drv, ST7920_CMD_SET_CGRAM_ADDR);//CGRAM 주소 초기화
    st7920_delay_ms(1);
    
    st7920_send_cmd(drv, ST7920_CMD_SET_DDRAM_ADDR);//DDRAM 주소 초기화
    st7920_delay_ms(1);

    
    // 8단계: 홈 위치로 이동
    // st7920_send_cmd(drv, ST7920_CMD_RETURN_HOME);
    // st7920_delay_ms(2);

    st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT | ST7920_FUNCTION_SET_EXTEND | ST7920_FUNCTION_SET_GRAPHIC);


    st7920_set_graphic_mode(drv, true);

    st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CLEAR);
    st7920_delay_ms(50);

       

    cfg->initialized = true;
}

void st7920_send_byte(driver_t *drv, uint8_t sync, uint8_t data)
{

    st7920_t *cfg = (st7920_t *)drv->cfg;
    
    // ST7920 시리얼 통신 시퀀스 - 참고 라이브러리 기반
    bsp_do_high(cfg->cs_do_num);  // CS HIGH (활성화)
    st7920_delay_us(1);
    
    // 3바이트 시리얼 프로토콜
    bsp_spi_send_byte(cfg->spi_num, sync);                    // 동기 바이트 (0xF8 or 0xFA)
    bsp_spi_send_byte(cfg->spi_num, data & 0xF0);            // 상위 4비트
    bsp_spi_send_byte(cfg->spi_num, (data << 4) & 0xF0);     // 하위 4비트
    
    bsp_do_low(cfg->cs_do_num);   // CS LOW (비활성화)
    st7920_delay_us(100);        // 명령 처리 대기

}


void st7920_send_data(driver_t *drv, uint8_t data)
{
#if ST7920_SPI_USE 
    st7920_send_byte(drv, ST7920_SYNC_DATA, data);
#endif
    
#if ST7920_GPIO_USE
    st7920_gpio_write_byte(data, false);  // false = data mode
    st7920_delay_us(72);  // ST7920 data write time
#endif
    
#if ST7920_MEM_USE

    *p_lcd_data = data;

    st7920_delay_us(72);  // ST7920 data write time (typical)
#endif
}

uint8_t reverse_bits(uint8_t b)
{
  uint8_t reversed_b = 0;
  for (int i = 0; i < 8; i++)
  {
    reversed_b <<= 1;  // 결과 비트를 왼쪽으로 한 칸 이동
    if (b & 1)         // 원본의 최하위 비트가 1이면
    {
      reversed_b |= 1;  // 결과의 최하위 비트를 1로 설정
    }
    b >>= 1;  // 원본 비트를 오른쪽으로 한 칸 이동
  }
  return reversed_b;
}



void st7920_clear_screen(driver_t *drv)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;

    
  memset(framebuffer,0,sizeof(framebuffer));
  st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CLEAR);
  st7920_delay_ms(2);  // 클리어 명령은 1.6ms 필요

}

void st7920_home(driver_t *drv)
{
    st7920_send_cmd(drv, ST7920_CMD_RETURN_HOME);
    st7920_delay_ms(2);
}

void st7920_display_on(driver_t *drv)
{
   // st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CONTROL | ST7920_DISPLAY_DISPLAY_ON);
   st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT | ST7920_FUNCTION_SET_EXTEND | ST7920_FUNCTION_SET_GRAPHIC);
   st7920_delay_us(100);
}

void st7920_display_off(driver_t *drv)
{
   // st7920_send_cmd(drv, ST7920_CMD_DISPLAY_CONTROL);
   st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT | ST7920_FUNCTION_SET_EXTEND );
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
        st7920_send_cmd(drv, ST7920_CMD_FUNCTION_SET | ST7920_FUNCTION_SET_8BIT |ST7920_FUNCTION_SET_EXTEND | ST7920_FUNCTION_SET_GRAPHIC);
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


//주소체계가 특이하다. 다시 구현해야함
//0,0에 1234567890ABCDEF 출력하면
// 0 에 12345678
// 2에 90ABCDEF 가 출력된다.

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



void rotate_screen(int degree, int width, int height, uint8_t *screen)
{
  // 180도 회전만 처리합니다.
  if (degree != 180)
  {
    return;
  }

  int buffer_size = (width * height) / 8;

  // 버퍼의 바이트 순서 뒤집기 ---
  // 메모리 시작부터 절반까지만 순회하며 양 끝의 바이트를 교환합니다.
  for (int i = 0; i < buffer_size / 2; i++)
  {
    uint8_t temp = screen[i];
    screen[i] = screen[buffer_size - 1 - i];
    screen[buffer_size - 1 - i] = temp;
  }

  // 각 바이트의 비트 순서 뒤집기 ---
  // 모든 바이트를 순회하며 비트 순서를 뒤집습니다.
  for (int i = 0; i < buffer_size; i++)
  {
    screen[i] = reverse_bits(screen[i]);
  }
}


uint8_t g_y=0;
uint8_t g_x=0;
uint8_t g_break=0;
void test_pixel(driver_t *drv)
{
  
  while(g_break)
  {
    st7920_send_cmd(drv, 0x80 | g_y);  // Y 주소 설정
    st7920_send_cmd(drv, 0x80|g_x);    // X 주소 8로 설정
    st7920_send_data(drv, 0xFF);
    st7920_send_data(drv, 0x55);

  }
}


//#define SCREEN_ROTATION 
/**
 * @brief 프레임버퍼의 내용을 LCD 화면 전체에 올바르게 전송
 */
void st7920_flush_buffer(driver_t *drv)
{
    st7920_t *cfg = (st7920_t *)drv->cfg;
    if(!cfg->graphic_mode) return;

#ifdef SCREEN_ROTATION
   rotate_screen(180, ST7920_WIDTH , ST7920_HEIGHT, (uint8_t *)framebuffer);
#endif
test_pixel(drv);
    //이상 증상 0번줄에 출력시 16번줄에 나타남
    // 하단 영역 (Y: 32~63)

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
    

#ifdef SCREEN_ROTATION
   rotate_screen(180, ST7920_WIDTH, ST7920_HEIGHT, (uint8_t *)framebuffer);
#endif
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

void st7920_put_ch(driver_t *drv, int row, int col, uint8_t ch)
{


  if (ch < ' ' || ch > '~')
    return;


  int start_x = 1 + col * FONT_6X8_WIDTH;
  int start_y = row * FONT_6X8_HEIGHT;


  if (start_x + FONT_6X8_WIDTH > ST7920_WIDTH || start_y + FONT_6X8_HEIGHT > ST7920_HEIGHT)
    return;


  const uint8_t *char_data = font_6x8[ch - 0x20];


  for (int y = 0; y < FONT_6X8_HEIGHT; y++)
  {
    uint8_t row_data = char_data[y];
    for (int x = 0; x < FONT_6X8_WIDTH; x++)
    {
      if (row_data & (0x10 >> x))  // Check bit from bit 4 (6-bit font uses bits 4-0)
      {
        st7920_set_pixel(drv,start_x + x, start_y + y, 1);
      }
      else
      {
        st7920_set_pixel(drv,start_x + x, start_y + y, 0);
      }
    }
  }
}


uint8_t *p_back_framebuffer=NULL;

void st7920_backup_framebuffer(void)
{
    if(p_back_framebuffer == NULL)
    {
        p_back_framebuffer = user_malloc(sizeof(framebuffer));

    
    }

    if(p_back_framebuffer)
    {
        memcpy(p_back_framebuffer,framebuffer,sizeof(framebuffer));
    }
}

void st7920_restore_framebuffer(void)
{
    if (p_back_framebuffer)
    {
        memcpy(framebuffer, p_back_framebuffer, sizeof(framebuffer));

    }
}


