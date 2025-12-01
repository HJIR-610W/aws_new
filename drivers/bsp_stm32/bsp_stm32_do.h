
#ifndef STM32_DO_H
#define STM32_DO_H

#include "pcb_define.h"

#ifdef PCB_0_5
#define STM32_DO_POWER_CDMA              0
#define STM32_DO_POWER_HART_24V          1
#define STM32_DO_POWER_RAIN_DECT_DIGITAL 2
#define STM32_DO_POWER_RAIN_DECT_ANALOG  3
#define STM32_DO_ADC_CS                  4
#define STM32_DO_FRAM_CS 5
#define STM32_DO_RTC_CS 6
#define STM32_DO_FLASH_CS 7      
#define STM32_DO_DIR_SDI 8      
#define STM32_DO_DIR_RS485_A  9  
#define STM32_DO_DIR_RS485_B 10
#define STM32_DO_DIR_RS485_C 11
#define STM32_DO_DIR_RS485_D 12
#define STM32_DO_HART_SEL    13     
#define STM32_DO_HART_RTS    14    
#define STM32_DO_HART_RESET  15
#define STM32_DO_LCD_RESET   16
#define STM32_DO_POWER_LCD  17
#define STM32_DO_POWER_BTM   18
#define STM32_DO_QUAD_A_RST  19
#define STM32_DO_QUAD_B_RST  20

#define STM32_DO_MAX 21
#endif

#ifdef PCB_0_6
#define STM32_DO_POWER_CDMA              0
#define STM32_DO_POWER_HART_24V          1
#define STM32_DO_POWER_RAIN_DECT_DIGITAL 2
#define STM32_DO_POWER_RAIN_DECT_ANALOG  3
#define STM32_DO_ADC_CS                  4
#define STM32_DO_FRAM_CS 5
#define STM32_DO_RTC_CS 6
#define STM32_DO_FLASH_CS 7      
#define STM32_DO_DIR_SDI 8      
#define STM32_DO_DIR_RS485_A  9  
#define STM32_DO_DIR_RS485_B 10
#define STM32_DO_DIR_RS485_C 11
#define STM32_DO_DIR_RS485_D 12
#define STM32_DO_HART_SEL    13     
#define STM32_DO_HART_RTS    14    
#define STM32_DO_HART_RESET  15
#define STM32_DO_LCD_RESET   16
#define STM32_DO_POWER_LCD  17
#define STM32_DO_QUAD_A_RST  18
#define STM32_DO_QUAD_B_RST  19

#define STM32_DO_MAX 20
#endif

void stm32_do_init(void);
void stm32_do_low(int num);
void stm32_do_high(int num);

#endif