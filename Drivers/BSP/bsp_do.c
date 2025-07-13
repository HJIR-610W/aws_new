#include "bsp.h"
#include "bsp_di.h"
#include "pcf8575.h"
#include "util_memory.h"

typedef struct bsp_do_inst_s
{
  GPIO_InitTypeDef init;
  GPIO_TypeDef *port;
} bsp_do_inst_t;

const bsp_do_inst_t do_inst[BSP_DI_MCU_MAX] = {
    [BSP_DI_USER_BTN] = {.init = {.Pin = DI_SW_SYS_Pin, .Pull = GPIO_PULLUP},
                         .port = DI_SW_SYS_GPIO_Port}};

void bsp_do_gpio_init(int do_number)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  board_clk_gpio(do_inst[do_number].port);

  GPIO_InitStruct.Pin = do_inst[do_number].init.Pin;
  GPIO_InitStruct.Mode = do_inst[do_number].init.Mode;
  GPIO_InitStruct.Pull = do_inst[do_number].init.Pull;
  HAL_GPIO_Init(do_inst[do_number].port, &GPIO_InitStruct);
}

void bsp_do_init(void)
{
  for (int do_num = 0; do_num < BSP_DO_MAX; do_num++)
  {
    switch (do_num)
    {
      case BSP_DO_POWER_CDMA:
      case BSP_DO_POWER_HART_24V:
      case BSP_DO_LCD_RESET:
      case BSP_DO_POWER_RAIN_DECT_DIGITAL:
      case BSP_DO_POWER_RAIN_DECT_ANALOG:
      case BSP_DO_ADC_NCS:
      case BSP_DO_FRAM_CS:
      case BSP_DO_RTC_CS:
      case BSP_DO_FLASH_CS:
      case BSP_DO_DIR_SDI:
      case BSP_DO_DIR_RS485_A:
      case BSP_DO_DIR_RS485_B:
      case BSP_DO_HART_SEL:
      case BSP_DO_HART_RTS:
      case BSP_DO_HART_RESET:
      case BSP_DO_DIR_RS485_C:
      case BSP_DO_DIR_RS485_D:
        bsp_do_gpio_init(do_num);
        break;
      case BSP_DO_EXT_0:  // App 정의되지 않음
      case BSP_DO_EXT_1:  // App 정의되지 않음
      case BSP_DO_EXT_2:  // App 정의되지 않음
      case BSP_DO_EXT_3:  // App 정의되지 않음
      case BSP_DO_EXT_4:  // App 정의되지 않음
      case BSP_DO_EXT_5:  // App 정의되지 않음
        pcf8575_init();
        break;

      default:
        break;
    }
  }
}

void bsp_do_low(int num)
{
  switch (num)
  {
    case BSP_DO_POWER_CDMA:
    case BSP_DO_POWER_HART_24V:
    case BSP_DO_LCD_RESET:
    case BSP_DO_POWER_RAIN_DECT_DIGITAL:
    case BSP_DO_POWER_RAIN_DECT_ANALOG:
    case BSP_DO_ADC_NCS:
    case BSP_DO_FRAM_CS:
    case BSP_DO_RTC_CS:
    case BSP_DO_FLASH_CS:
    case BSP_DO_DIR_SDI:
    case BSP_DO_DIR_RS485_A:
    case BSP_DO_DIR_RS485_B:
    case BSP_DO_HART_SEL:
    case BSP_DO_HART_RTS:
    case BSP_DO_HART_RESET:
    case BSP_DO_DIR_RS485_C:
    case BSP_DO_DIR_RS485_D:
      HAL_GPIO_WritePin(do_inst[num].port, do_inst[num].init.Pin, GPIO_PIN_RESET);
      break;
    case BSP_DO_EXT_0:  
     pcf8575_write_pin(DO_PCF8575_0, 0);
     break;
    case BSP_DO_EXT_1: 
      pcf8575_write_pin(DO_PCF8575_1, 0);
      break;
    case BSP_DO_EXT_2: 
      pcf8575_write_pin(DO_PCF8575_2, 0);
      break;
    case BSP_DO_EXT_3:  
      pcf8575_write_pin(DO_PCF8575_3, 0);
      break;
    case BSP_DO_EXT_4:  
      pcf8575_write_pin(DO_PCF8575_5, 0);
      break;
    case BSP_DO_EXT_5: 
      pcf8575_write_pin(DO_PCF8575_6, 0);
      break;

    default:
      break;
  }
}

void bsp_do_high(int num)
{
  switch (num)
  {
    case BSP_DO_POWER_CDMA:
    case BSP_DO_POWER_HART_24V:
    case BSP_DO_LCD_RESET:
    case BSP_DO_POWER_RAIN_DECT_DIGITAL:
    case BSP_DO_POWER_RAIN_DECT_ANALOG:
    case BSP_DO_ADC_NCS:
    case BSP_DO_FRAM_CS:
    case BSP_DO_RTC_CS:
    case BSP_DO_FLASH_CS:
    case BSP_DO_DIR_SDI:
    case BSP_DO_DIR_RS485_A:
    case BSP_DO_DIR_RS485_B:
    case BSP_DO_HART_SEL:
    case BSP_DO_HART_RTS:
    case BSP_DO_HART_RESET:
    case BSP_DO_DIR_RS485_C:
    case BSP_DO_DIR_RS485_D:
      HAL_GPIO_WritePin(do_inst[num].port, do_inst[num].init.Pin, GPIO_PIN_SET);
      break;
    case BSP_DO_EXT_0:
      pcf8575_write_pin(DO_PCF8575_0, 1);
      break;
    case BSP_DO_EXT_1:
      pcf8575_write_pin(DO_PCF8575_1, 1);
      break;
    case BSP_DO_EXT_2:
      pcf8575_write_pin(DO_PCF8575_2, 1);
      break;
    case BSP_DO_EXT_3:
      pcf8575_write_pin(DO_PCF8575_3, 1);
      break;
    case BSP_DO_EXT_4:
      pcf8575_write_pin(DO_PCF8575_5, 1);
      break;
    case BSP_DO_EXT_5:
      pcf8575_write_pin(DO_PCF8575_6, 1);
      break;

    default:
      break;
  }
}