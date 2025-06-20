
#ifndef MCU_UTILE_H
#define MCU_UTILE_H

#include "stm32f4xx_hal.h"


void board_clk_gpio(GPIO_TypeDef *GPIOx);

void board_set_gpio(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void board_config_gpio(GPIO_TypeDef *GPIOx,uint32_t pin,uint32_t mode,uint32_t pull,uint32_t speed,uint32_t alternate);

uint32_t get_apb2_timer_clock(void);
#endif
