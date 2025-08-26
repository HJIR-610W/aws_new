#include "bsp_do.h"

#include "bsp.h"
#include "pcf8575.h"
#include "util_memory.h"
#include "pcb_define.h"
#include "dev_io.h"
#include "driver_stm32_do.h"

typedef enum
{
  DO_DRIVER_STM32,
  DO_DRIVER_PCF8575,
  DO_DRIVER_INVALID
} do_driver_type_t;

typedef struct
{
  do_driver_type_t driver_type;
  int driver_num;
} do_pinmap_t;

#ifdef AWS_PCB_0_5
static const do_pinmap_t do_pinmap[BSP_DO_MAX] = {
    [BSP_DO_POWER_CDMA] = {DO_DRIVER_STM32, STM32_DO_POWER_CDMA},
    [BSP_DO_POWER_HART_24V] = {DO_DRIVER_STM32, STM32_DO_POWER_HART_24V},
    [BSP_DO_POWER_RAIN_DECT_DIGITAL] = {DO_DRIVER_STM32, STM32_DO_POWER_RAIN_DECT_DIGITAL},
    [BSP_DO_POWER_RAIN_DECT_ANALOG] = {DO_DRIVER_STM32, STM32_DO_POWER_RAIN_DECT_ANALOG},
    [BSP_DO_ADC_CS] = {DO_DRIVER_STM32, STM32_DO_ADC_CS},
    [BSP_DO_FRAM_CS] = {DO_DRIVER_STM32, STM32_DO_FRAM_CS},
    [BSP_DO_RTC_CS] = {DO_DRIVER_STM32, STM32_DO_RTC_CS},
    [BSP_DO_FLASH_CS] = {DO_DRIVER_STM32, STM32_DO_FLASH_CS},
    [BSP_DO_DIR_SDI] = {DO_DRIVER_STM32, STM32_DO_DIR_SDI},
    [BSP_DO_DIR_RS485_A] = {DO_DRIVER_STM32, STM32_DO_DIR_RS485_A},
    [BSP_DO_DIR_RS485_B] = {DO_DRIVER_STM32, STM32_DO_DIR_RS485_B},
    [BSP_DO_DIR_RS485_C] = {DO_DRIVER_STM32, STM32_DO_DIR_RS485_C},
    [BSP_DO_DIR_RS485_D] = {DO_DRIVER_STM32, STM32_DO_DIR_RS485_D},
    [BSP_DO_HART_SEL] = {DO_DRIVER_STM32, STM32_DO_HART_SEL},
    [BSP_DO_HART_RTS] = {DO_DRIVER_STM32, STM32_DO_HART_RTS},
    [BSP_DO_HART_RESET] = {DO_DRIVER_STM32, STM32_DO_HART_RESET},
    [BSP_DO_LCD_RESET] = {DO_DRIVER_STM32, STM32_DO_LCD_RESET},
    [BSP_DO_POWER_LCD] = {DO_DRIVER_STM32, STM32_DO_POWER_LCD},
    [BSP_DO_POWER_BTM] = {DO_DRIVER_STM32, STM32_DO_POWER_BTM},
    [BSP_DO_QUAD_A_RST] = {DO_DRIVER_STM32, STM32_DO_QUAD_A_RST},
    [BSP_DO_QUAD_B_RST] = {DO_DRIVER_STM32, STM32_DO_QUAD_B_RST},
    [BSP_DO_EXT_0] = {DO_DRIVER_PCF8575, DO_PCF8575_0},
    [BSP_DO_EXT_1] = {DO_DRIVER_PCF8575, DO_PCF8575_1},
    [BSP_DO_EXT_2] = {DO_DRIVER_PCF8575, DO_PCF8575_2},
    [BSP_DO_EXT_3] = {DO_DRIVER_PCF8575, DO_PCF8575_3},
    [BSP_DO_EXT_4] = {DO_DRIVER_PCF8575, DO_PCF8575_5},
    [BSP_DO_EXT_5] = {DO_DRIVER_PCF8575, DO_PCF8575_6}};
#endif


#ifdef AWS_PCB_0_6
static const do_pinmap_t do_pinmap[BSP_DO_MAX] = {
    [BSP_DO_POWER_CDMA] = {DO_DRIVER_STM32, STM32_DO_POWER_CDMA},
    [BSP_DO_POWER_HART_24V] = {DO_DRIVER_STM32, STM32_DO_POWER_HART_24V},
    [BSP_DO_POWER_RAIN_DECT_DIGITAL] = {DO_DRIVER_STM32, STM32_DO_POWER_RAIN_DECT_DIGITAL},
    [BSP_DO_POWER_RAIN_DECT_ANALOG] = {DO_DRIVER_STM32, STM32_DO_POWER_RAIN_DECT_ANALOG},
    [BSP_DO_ADC_CS] = {DO_DRIVER_STM32, STM32_DO_ADC_CS},
    [BSP_DO_FRAM_CS] = {DO_DRIVER_STM32, STM32_DO_FRAM_CS},
    [BSP_DO_RTC_CS] = {DO_DRIVER_STM32, STM32_DO_RTC_CS},
    [BSP_DO_FLASH_CS] = {DO_DRIVER_STM32, STM32_DO_FLASH_CS},
    [BSP_DO_DIR_SDI] = {DO_DRIVER_STM32, STM32_DO_DIR_SDI},
    [BSP_DO_DIR_RS485_A] = {DO_DRIVER_STM32, STM32_DO_DIR_RS485_A},
    [BSP_DO_DIR_RS485_B] = {DO_DRIVER_STM32, STM32_DO_DIR_RS485_B},
    [BSP_DO_DIR_RS485_C] = {DO_DRIVER_STM32, STM32_DO_DIR_RS485_C},
    [BSP_DO_DIR_RS485_D] = {DO_DRIVER_STM32, STM32_DO_DIR_RS485_D},
    [BSP_DO_HART_SEL] = {DO_DRIVER_STM32, STM32_DO_HART_SEL},
    [BSP_DO_HART_RTS] = {DO_DRIVER_STM32, STM32_DO_HART_RTS},
    [BSP_DO_HART_RESET] = {DO_DRIVER_STM32, STM32_DO_HART_RESET},
    [BSP_DO_LCD_RESET] = {DO_DRIVER_STM32, STM32_DO_LCD_RESET},
    [BSP_DO_POWER_LCD] = {DO_DRIVER_STM32, STM32_DO_POWER_LCD},
    [BSP_DO_QUAD_A_RST] = {DO_DRIVER_STM32, STM32_DO_QUAD_A_RST},
    [BSP_DO_QUAD_B_RST] = {DO_DRIVER_STM32, STM32_DO_QUAD_B_RST},
    [BSP_DO_EXT_0] = {DO_DRIVER_PCF8575, DO_PCF8575_0},
    [BSP_DO_EXT_1] = {DO_DRIVER_PCF8575, DO_PCF8575_1},
    [BSP_DO_EXT_2] = {DO_DRIVER_PCF8575, DO_PCF8575_2},
    [BSP_DO_EXT_3] = {DO_DRIVER_PCF8575, DO_PCF8575_3},
    [BSP_DO_EXT_4] = {DO_DRIVER_PCF8575, DO_PCF8575_5},
    [BSP_DO_EXT_5] = {DO_DRIVER_PCF8575, DO_PCF8575_6}};
#endif

static inline bool is_valid_do_num(int num)
{
  return (num >= 0 && num < BSP_DO_MAX);
}

static inline const do_pinmap_t *get_do_pinmap(int num)
{
  if (!is_valid_do_num(num))
  {
    return 0;
  }
  return &do_pinmap[num];
}




void bsp_do_init(void)
{

        stm32_do_init();

        pcf8575_init();

}

void bsp_do_low(int num)
{
  const do_pinmap_t* pinmap = get_do_pinmap(num);
  if (!pinmap) return;

  switch (pinmap->driver_type)
  {
    case DO_DRIVER_STM32:
      stm32_do_low(pinmap->driver_num);
      break;
    case DO_DRIVER_PCF8575:
      pcf8575_write_pin(pinmap->driver_num, 0);
      break;
    default:
      break;
  }
}

void bsp_do_high(int num)
{
  const do_pinmap_t* pinmap = get_do_pinmap(num);
  if (!pinmap) return;

  switch (pinmap->driver_type)
  {
    case DO_DRIVER_STM32:
      stm32_do_high(pinmap->driver_num);
      break;
    case DO_DRIVER_PCF8575:
      pcf8575_write_pin(pinmap->driver_num, 1);
      break;
    default:
      break;
  }
}