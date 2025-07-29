

#ifndef BSP_DI_H
#define BSP_DI_H


#include <stdint.h>
#include <stdbool.h>

#include "driver_di_def.h"

#define BSP_DI_0_ADC_RDY_ONLY 0
#define BSP_DI_HART_CD_ONLY   1
#define BSP_DI_USER_BTN       2
#define BSP_DI_RAIN_REED      3
#define BSP_DI_RAIN_HALL      4
#define BSP_DI_RAIN_HALL_ERR  5
#define BSP_DI_RAIN_DETECT_A  6

#define BSP_DI_QUAD_UARTA_1 7
#define BSP_DI_QUAD_UARTB_2 8
#define BSP_DI_QUAD_UARTC_3 9
#define BSP_DI_QUAD_UARTD_4 10
#define BSP_DI_QUAD_UARTA_5 11
#define BSP_DI_QUAD_UARTB_6 12
#define BSP_DI_QUAD_UARTC_7 13
#define BSP_DI_QUAD_UARTD_8 14

#define BSP_DI_0 15
#define BSP_DI_1 16
#define BSP_DI_2 17
#define BSP_DI_3 18
#define BSP_DI_4 19
#define BSP_DI_5 20

#define BSP_DI_WAKE_UP        21
#define BSP_DI_BOOT1          22 
#define BSP_DI_HART_CD        23
#define BSP_DI_RTC_INT        24
#define BSP_DI_SD_IN          25
#define BSP_DI_USB_POWER_FAIL 26
#define BSP_DI_IO_INT         27

#define BSP_DI_MCU_MAX        28



void bsp_di_init(void);
int32_t bsp_di_read(int32_t num);

void bsp_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg);
#endif
