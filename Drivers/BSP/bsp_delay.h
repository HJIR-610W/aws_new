

#ifndef MCU_DELAY_H
#define MCU_DELAY_H

void bsp_delay_init(void) ;
void bsp_us_delay(uint32_t us);

uint32_t mcu_get_clk(void);
uint32_t cal_elapsed_us(uint32_t start);
#endif
