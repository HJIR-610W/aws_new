

#include "bsp_di.h"

#include "bsp.h"
#include "pcf8575.h"
#include "util_memory.h"
#include "pcb_define.h"
#include "driver_stm32_di.h"

// 드라이버 타입 정의
typedef enum
{
  DI_DRIVER_STM32,
  DI_DRIVER_PCF8575,
  UART_DRIVER_INVALID
} di_driver_type_t;


typedef struct
{
  di_driver_type_t driver_type;
  int driver_num;
} di_pinmap_t;


#ifdef PCB_0_5
static const di_pinmap_t di_pinmap[BSP_DI_MCU_MAX] = {
    [BSP_DI_USER_BTN] = {DI_DRIVER_STM32, STM32_DI_USER_BTN},
    [BSP_DI_RAIN_REED] = {DI_DRIVER_STM32, STM32_DI_RAIN_REED},
    [BSP_DI_RAIN_HALL] = {DI_DRIVER_STM32, STM32_DI_RAIN_HALL},
    [BSP_DI_RAIN_HALL_ERR] = {DI_DRIVER_STM32, STM32_DI_RAIN_HALL_ERR},
    [BSP_DI_RAIN_DETECT_A] = {DI_DRIVER_STM32, STM32_DI_RAIN_DETECT_A},
    [BSP_DI_QUAD_UARTA_1] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTA_1},
    [BSP_DI_QUAD_UARTB_2] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTB_2},
    [BSP_DI_QUAD_UARTC_3] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTC_3},
    [BSP_DI_QUAD_UARTD_4] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTD_4},
    [BSP_DI_QUAD_UARTA_5] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTA_5},
    [BSP_DI_QUAD_UARTB_6] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTB_6},
    [BSP_DI_QUAD_UARTC_7] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTC_7},
    [BSP_DI_QUAD_UARTD_8] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTD_8},
    [BSP_DI_WAKE_UP] = {DI_DRIVER_STM32, STM32_DI_WAKE_UP},
    [BSP_DI_BOOT1] = {DI_DRIVER_STM32, STM32_DI_BOOT1},
    [BSP_DI_HART_CD] = {DI_DRIVER_STM32, STM32_DI_HART_CD},
    [BSP_DI_RTC_INT] = {DI_DRIVER_STM32, STM32_DI_RTC_INT},
    [BSP_DI_0_ADC_RDY] = {DI_DRIVER_STM32, STM32_DI_0_ADC_RDY},
    [BSP_DI_IO_INT] = {DI_DRIVER_STM32, STM32_DI_IO_INT},
    [BSP_DI_SD_IN] = {DI_DRIVER_STM32, STM32_DI_SD_IN},
    [BSP_DI_USB_POWER_FAIL] = {DI_DRIVER_STM32, STM32_DI_USB_POWER_FAIL},
    [BSP_DI_0] = {DI_DRIVER_PCF8575, DI_PCF8575_0},
    [BSP_DI_1] = {DI_DRIVER_PCF8575, DI_PCF8575_1},
    [BSP_DI_2] = {DI_DRIVER_PCF8575, DI_PCF8575_2},
    [BSP_DI_3] = {DI_DRIVER_PCF8575, DI_PCF8575_3},
    [BSP_DI_4] = {DI_DRIVER_PCF8575, DI_PCF8575_4},
    [BSP_DI_5] = {DI_DRIVER_PCF8575, DI_PCF8575_5}};

#endif

#ifdef PCB_0_6
static const di_pinmap_t di_pinmap[BSP_DI_MCU_MAX] = {
    [BSP_DI_USER_BTN] = {DI_DRIVER_STM32, STM32_DI_USER_BTN},
    [BSP_DI_RAIN_REED] = {DI_DRIVER_STM32, STM32_DI_RAIN_REED},
    [BSP_DI_RAIN_HALL] = {DI_DRIVER_STM32, STM32_DI_RAIN_HALL},
    [BSP_DI_RAIN_HALL_ERR] = {DI_DRIVER_STM32, STM32_DI_RAIN_HALL_ERR},
    [BSP_DI_RAIN_DETECT_A] = {DI_DRIVER_STM32, STM32_DI_RAIN_DETECT_A},
    [BSP_DI_QUAD_UARTA_1] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTA_1},
    [BSP_DI_QUAD_UARTB_2] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTB_2},
    [BSP_DI_QUAD_UARTC_3] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTC_3},
    [BSP_DI_QUAD_UARTD_4] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTD_4},
    [BSP_DI_QUAD_UARTA_5] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTA_5},
    [BSP_DI_QUAD_UARTB_6] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTB_6},
    [BSP_DI_QUAD_UARTC_7] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTC_7},
    [BSP_DI_QUAD_UARTD_8] = {DI_DRIVER_STM32, STM32_DI_QUAD_UARTD_8},
    [BSP_DI_BOOT1] = {DI_DRIVER_STM32, STM32_DI_BOOT1},
    [BSP_DI_HART_CD] = {DI_DRIVER_STM32, STM32_DI_HART_CD},
    [BSP_DI_RTC_INT] = {DI_DRIVER_STM32, STM32_DI_RTC_INT},
    [BSP_DI_0_ADC_RDY] = {DI_DRIVER_STM32, STM32_DI_0_ADC_RDY},
    [BSP_DI_IO_INT] = {DI_DRIVER_STM32, STM32_DI_IO_INT},
    [BSP_DI_SD_IN] = {DI_DRIVER_STM32, STM32_DI_SD_IN},
    [BSP_DI_USB_POWER_FAIL] = {DI_DRIVER_STM32, STM32_DI_USB_POWER_FAIL},
    [BSP_DI_0] = {DI_DRIVER_PCF8575, DI_PCF8575_0},
    [BSP_DI_1] = {DI_DRIVER_PCF8575, DI_PCF8575_1},
    [BSP_DI_2] = {DI_DRIVER_PCF8575, DI_PCF8575_2},
    [BSP_DI_3] = {DI_DRIVER_PCF8575, DI_PCF8575_3},
    [BSP_DI_4] = {DI_DRIVER_PCF8575, DI_PCF8575_4},
    [BSP_DI_5] = {DI_DRIVER_PCF8575, DI_PCF8575_5}};

#endif

static inline bool is_valid_di_num(int num)
{
  return (num >= 0 && num < BSP_DI_MCU_MAX);
}

static inline const di_pinmap_t *get_di_pinmap(int num)
{
  if (!is_valid_di_num(num))
  {
    return 0;
  }
  return &di_pinmap[num];
}

void bsp_di_init(void)
{
  stm32_di_init();
  pcf8575_init();
}

int32_t bsp_di_read(int32_t di_number)
{
  const di_pinmap_t* pinmap = get_di_pinmap(di_number);
  if (!pinmap) return -99;

  switch (pinmap->driver_type)
  {
    case DI_DRIVER_STM32:
      return stm32_di_read(pinmap->driver_num);
    case DI_DRIVER_PCF8575:
      return pcf8575_read_pin(pinmap->driver_num);
    default:
      break;
  }
  return -99;
}

void bsp_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg)
{
  const di_pinmap_t *pinmap = get_di_pinmap(di_number);
  if (!pinmap)
    return ;

  switch (pinmap->driver_type)
  {
  case DI_DRIVER_STM32:
    stm32_di_set_interrupt(pinmap->driver_num,isr_cfg);
    break;
  case DI_DRIVER_PCF8575:
  default:
    break;
  }

}
