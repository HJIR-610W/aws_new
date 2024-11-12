

#ifndef MCU_DELAY_H
#define MCU_DELAY_H

void DWT_Delay_Init(void) ;
void mcu_delay(uint32_t us);

uint32_t mcu_get_clk(void);
uint32_t mcu_cal_elapse_us(uint32_t start);
#endif
