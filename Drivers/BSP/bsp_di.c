

#include "bsp_di.h"

#include "bsp.h"
#include "pcf8575.h"
#include "util_memory.h"


typedef struct bsp_di_inst_s
{
  GPIO_InitTypeDef init;
  GPIO_TypeDef *port;
} bsp_di_inst_t;

const bsp_di_inst_t di_inst[BSP_DI_MCU_MAX] = {
    [BSP_DI_USER_BTN] = {.init = {.Pin = DI_SW_SYS_Pin, .Pull = GPIO_PULLUP},
                         .port = DI_SW_SYS_GPIO_Port}};

void bsp_di_gpio_init(int di_number)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  board_clk_gpio(di_inst[di_number].port);

  GPIO_InitStruct.Pin = di_inst[di_number].init.Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = di_inst[di_number].init.Pull;
  HAL_GPIO_Init(di_inst[di_number].port, &GPIO_InitStruct);
}

void bsp_di_init(void)
{
  for (int di_num = 0; di_num < BSP_DI_MCU_MAX; di_num++)
  {
    switch (di_num)
    {
      case BSP_DI_USER_BTN:
      case BSP_DI_RAIN_REED:
      case BSP_DI_RAIN_HALL:
      case BSP_DI_RAIN_HALL_ERR:
      case BSP_DI_RAIN_DETECT:
      case BSP_DI_QUAD_UARTA_1:
      case BSP_DI_QUAD_UARTB_2:
      case BSP_DI_QUAD_UARTC_3:
      case BSP_DI_QUAD_UARTD_4:
      case BSP_DI_QUAD_UARTA_5:
      case BSP_DI_QUAD_UARTB_6:
      case BSP_DI_QUAD_UARTC_7:
      case BSP_DI_QUAD_UARTD_8:
        bsp_di_gpio_init(di_num);
        break; 
      case BSP_DI_0:
      case BSP_DI_1:
      case BSP_DI_2:
      case BSP_DI_3:
      case BSP_DI_4:
      case BSP_DI_5:
      case BSP_DI_6:
      case BSP_DI_7:
       pcf8575_init();
      break;

      default:
        break;
    }
  }
}

int32_t bsp_di_read(int32_t di_number)
{
  switch (di_number)
  {
    case BSP_DI_USER_BTN:
      return HAL_GPIO_ReadPin(di_inst[di_number].port, di_inst[di_number].init.Pin);
      break;
    case BSP_DI_0:
      return pcf8575_read_pin(DI_PCF8575_0);
      break; 
    case BSP_DI_1:
      return pcf8575_read_pin(DI_PCF8575_1);
      break;
    case BSP_DI_2:
      return pcf8575_read_pin(DI_PCF8575_2);
      break;
    case BSP_DI_3:
      return pcf8575_read_pin(DI_PCF8575_3);
      break;
    case BSP_DI_4:
      return pcf8575_read_pin(DI_PCF8575_4);
      break;
    case BSP_DI_5:
      return pcf8575_read_pin(DI_PCF8575_5);
      break;
    case BSP_DI_6:
      return pcf8575_read_pin(DI_PCF8575_6);
      break;
    case BSP_DI_7:
      return pcf8575_read_pin(DI_PCF8575_7);
      break;
    default:
      break;
  }
}

void bsp_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg)
{
  uint16_t pin;
  switch (di_number)
  {
    case BSP_DI_USER_BTN:
    case BSP_DI_RAIN_REED:
    case BSP_DI_RAIN_HALL:
    case BSP_DI_RAIN_HALL_ERR:
    case BSP_DI_RAIN_DETECT:
    case BSP_DI_QUAD_UARTA_1:
    case BSP_DI_QUAD_UARTB_2:
    case BSP_DI_QUAD_UARTC_3:
    case BSP_DI_QUAD_UARTD_4:
    case BSP_DI_QUAD_UARTA_5:
    case BSP_DI_QUAD_UARTB_6:
    case BSP_DI_QUAD_UARTC_7:
    case BSP_DI_QUAD_UARTD_8:
    {
      exti_isr_cfg_t exti_isr_cfg;
      di_isr_set_cfg_t *isr;
      isr = (di_isr_set_cfg_t *)isr_cfg;
      pin = di_inst[di_number].init.Pin;

      __HAL_GPIO_EXTI_CLEAR_IT(pin);
      exti_isr_cfg.irq = get_irqFromPin(pin);
      exti_isr_cfg.call = isr->call;
      exti_isr_cfg.name = isr->name;
      exti_isr_cfg.gpio_pin = pin;
      exti_isr_cfg.handle = isr->handle;
      exti_register(&exti_isr_cfg);

      GPIO_InputInterrupt_Init(di_inst[di_number].port,pin, isr->trigger, isr->prio);
      }
    }
 


}