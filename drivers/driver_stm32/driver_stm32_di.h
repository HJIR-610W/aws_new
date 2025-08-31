

#ifndef STM32_DI_H
#define STM32_DI_H

#include <stdint.h>
#include <stdbool.h>

#include "pcb_define.h"
#include "driver_di_def.h"

#ifdef PCB_0_5

#define STM32_DI_USER_BTN 0
#define STM32_DI_RAIN_REED 1
#define STM32_DI_RAIN_HALL 2
#define STM32_DI_RAIN_HALL_ERR 3
#define STM32_DI_RAIN_DETECT_A 4
#define STM32_DI_QUAD_UARTA_1 5
#define STM32_DI_QUAD_UARTB_2 6
#define STM32_DI_QUAD_UARTC_3 7
#define STM32_DI_QUAD_UARTD_4 8
#define STM32_DI_QUAD_UARTA_5 9
#define STM32_DI_QUAD_UARTB_6 10
#define STM32_DI_QUAD_UARTC_7 11
#define STM32_DI_QUAD_UARTD_8 12
#define STM32_DI_WAKE_UP 13 // 미사용
#define STM32_DI_BOOT1 14   // 미사용
#define STM32_DI_HART_CD 15
#define STM32_DI_RTC_INT 16
#define STM32_DI_0_ADC_RDY 17
#define STM32_DI_IO_INT 18
#define STM32_DI_SD_IN 19
#define STM32_DI_USB_POWER_FAIL 20
#define STM32_DI_MCU_MAX 21

#endif

#ifdef PCB_0_6

#define STM32_DI_USER_BTN 0
#define STM32_DI_RAIN_REED 1
#define STM32_DI_RAIN_HALL 2
#define STM32_DI_RAIN_HALL_ERR 3
#define STM32_DI_RAIN_DETECT_A 4
#define STM32_DI_QUAD_UARTA_1 5
#define STM32_DI_QUAD_UARTB_2 6
#define STM32_DI_QUAD_UARTC_3 7
#define STM32_DI_QUAD_UARTD_4 8
#define STM32_DI_QUAD_UARTA_5 9
#define STM32_DI_QUAD_UARTB_6 10
#define STM32_DI_QUAD_UARTC_7 11
#define STM32_DI_QUAD_UARTD_8 12
#define STM32_DI_BOOT1 13   // 미사용
#define STM32_DI_HART_CD 14
#define STM32_DI_RTC_INT 15
#define STM32_DI_0_ADC_RDY 16
#define STM32_DI_IO_INT 17
#define STM32_DI_SD_IN 18
#define STM32_DI_USB_POWER_FAIL 19
#define STM32_DI_MCU_MAX 20

#endif

void stm32_di_init(void);
int32_t stm32_di_read(int32_t num);
void stm32_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg);
#endif
