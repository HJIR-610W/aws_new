

#ifndef BSP_DI_H
#define BSP_DI_H


#include <stdint.h>
#include <stdbool.h>

#include "driver_di_def.h"

#define BSP_DI_0_ADC_RDY_ONLY 0
#define BSP_DI_HART_CD_ONLY 1
#define BSP_DI_1_RTC_IRQ_ONLY 2
#define BSP_DI_USER_BTN 3
#define BSP_DI_RAIN_REED 4
#define BSP_DI_RAIN_HALL 5
#define BSP_DI_RAIN_HALL_ERR 6
#define BSP_DI_RAIN_DETECT 7

#define BSP_DI_QUAD_UARTA_1 8
#define BSP_DI_QUAD_UARTB_2 9
#define BSP_DI_QUAD_UARTC_3 10
#define BSP_DI_QUAD_UARTD_4 11
#define BSP_DI_QUAD_UARTA_5 12
#define BSP_DI_QUAD_UARTB_6 13
#define BSP_DI_QUAD_UARTC_7 14
#define BSP_DI_QUAD_UARTD_8 15

#define BSP_DI_0 16
#define BSP_DI_1 17
#define BSP_DI_2 18
#define BSP_DI_3 19
#define BSP_DI_4 20
#define BSP_DI_5 21




#define BSP_DI_MCU_MAX 26

void bsp_di_init(void);
int32_t bsp_di_read(int32_t num);

void bsp_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg);
#endif
