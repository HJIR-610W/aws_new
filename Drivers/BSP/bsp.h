

#ifndef BSP_H_
#define BSP_H_

#include <stdbool.h>
#include "pcb_define.h"
#include "bsp_do.h"
#include "bsp_di.h"
#include "bsp_rtc.h"

#define LED_BLINK 0
#define LED_ON 1


void bsp_init(void);
float bsp_read_battery(void);    // 보드 전원
float bsp_read_temperature(void);// 보드 온도
void board_clk_gpio(GPIO_TypeDef *GPIOx);
void board_set_gpio(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void board_config_gpio(GPIO_TypeDef *GPIOx,uint32_t pin,uint32_t mode,uint32_t pull,uint32_t speed,uint32_t alternate);
uint32_t get_apb2_timer_clock(void);

#endif