

#ifndef BSP_DI_H
#define BSP_DI_H


#include <stdint.h>
#include <stdbool.h>

#include "driver_di_def.h"
#include "pcb_define.h"

#ifdef PCB_0_5
#define BSP_DI_USER_BTN        0
#define BSP_DI_RAIN_REED       1
#define BSP_DI_RAIN_HALL       2
#define BSP_DI_RAIN_HALL_ERR   3
#define BSP_DI_RAIN_DETECT_A   4
#define BSP_DI_QUAD_UARTA_1    5
#define BSP_DI_QUAD_UARTB_2    6
#define BSP_DI_QUAD_UARTC_3    7
#define BSP_DI_QUAD_UARTD_4    8
#define BSP_DI_QUAD_UARTA_5    9
#define BSP_DI_QUAD_UARTB_6   10
#define BSP_DI_QUAD_UARTC_7   11
#define BSP_DI_QUAD_UARTD_8   12
#define BSP_DI_WAKE_UP        13 // 미사용
#define BSP_DI_BOOT1          14 // 미사용
#define BSP_DI_HART_CD        15
#define BSP_DI_RTC_INT        16
#define BSP_DI_0_ADC_RDY      17
#define BSP_DI_IO_INT         18
#define BSP_DI_SD_IN          19
#define BSP_DI_USB_POWER_FAIL 20
#define BSP_DI_0              21
#define BSP_DI_1              22
#define BSP_DI_2              23
#define BSP_DI_3              24
#define BSP_DI_4              25
#define BSP_DI_5              26
#define BSP_DI_MCU_MAX        27

#endif


#ifdef PCB_0_6
#define BSP_DI_USER_BTN        0
#define BSP_DI_RAIN_REED       1
#define BSP_DI_RAIN_HALL       2
#define BSP_DI_RAIN_HALL_ERR   3
#define BSP_DI_RAIN_DETECT_A   4
#define BSP_DI_QUAD_UARTA_1    5
#define BSP_DI_QUAD_UARTB_2    6
#define BSP_DI_QUAD_UARTC_3    7
#define BSP_DI_QUAD_UARTD_4    8
#define BSP_DI_QUAD_UARTA_5    9
#define BSP_DI_QUAD_UARTB_6   10
#define BSP_DI_QUAD_UARTC_7   11
#define BSP_DI_QUAD_UARTD_8   12
#define BSP_DI_BOOT1          13 // 미사용
#define BSP_DI_HART_CD        14
#define BSP_DI_RTC_INT        15
#define BSP_DI_0_ADC_RDY      16
#define BSP_DI_IO_INT         17
#define BSP_DI_SD_IN          18
#define BSP_DI_USB_POWER_FAIL 19
#define BSP_DI_0              20
#define BSP_DI_1              21
#define BSP_DI_2              22
#define BSP_DI_3              23
#define BSP_DI_4              24
#define BSP_DI_5              25
#define BSP_DI_MCU_MAX        26
#endif

void bsp_di_init(void);
int32_t bsp_di_read(int32_t num);
void bsp_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg);
#endif
