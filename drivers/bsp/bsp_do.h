



#ifndef BSP_DO_H
#define BSP_DO_H

#include "pcb_define.h"

#ifdef AWS_PCB_0_5
#define BSP_DO_POWER_CDMA              0
#define BSP_DO_POWER_HART_24V          1
#define BSP_DO_POWER_RAIN_DECT_DIGITAL 2
#define BSP_DO_POWER_RAIN_DECT_ANALOG  3
#define BSP_DO_ADC_CS                  4
#define BSP_DO_FRAM_CS                 5
#define BSP_DO_RTC_CS                  6
#define BSP_DO_FLASH_CS                7      
#define BSP_DO_DIR_SDI                 8      
#define BSP_DO_DIR_RS485_A             9  
#define BSP_DO_DIR_RS485_B            10
#define BSP_DO_DIR_RS485_C            11
#define BSP_DO_DIR_RS485_D            12
#define BSP_DO_HART_SEL               13     
#define BSP_DO_HART_RTS               14    
#define BSP_DO_HART_RESET             15
#define BSP_DO_LCD_RESET              16
#define BSP_DO_POWER_LCD             17
#define BSP_DO_POWER_BTM              18
#define BSP_DO_QUAD_A_RST             19
#define BSP_DO_QUAD_B_RST             20
#define BSP_DO_EXT_0                  21
#define BSP_DO_EXT_1                  22  
#define BSP_DO_EXT_2                  23 
#define BSP_DO_EXT_3                  24 
#define BSP_DO_EXT_4                  25 
#define BSP_DO_EXT_5                  26
#define BSP_DO_MAX                    27
#endif

#ifdef AWS_PCB_0_6
#define BSP_DO_POWER_CDMA              0
#define BSP_DO_POWER_HART_24V          1
#define BSP_DO_POWER_RAIN_DECT_DIGITAL 2
#define BSP_DO_POWER_RAIN_DECT_ANALOG  3
#define BSP_DO_ADC_CS                  4
#define BSP_DO_FRAM_CS                 5
#define BSP_DO_RTC_CS                  6
#define BSP_DO_FLASH_CS                7      
#define BSP_DO_DIR_SDI                 8      
#define BSP_DO_DIR_RS485_A             9  
#define BSP_DO_DIR_RS485_B            10
#define BSP_DO_DIR_RS485_C            11
#define BSP_DO_DIR_RS485_D            12
#define BSP_DO_HART_SEL               13     
#define BSP_DO_HART_RTS               14    
#define BSP_DO_HART_RESET             15
#define BSP_DO_LCD_RESET              16
#define BSP_DO_POWER_LCD              17
#define BSP_DO_QUAD_A_RST             18
#define BSP_DO_QUAD_B_RST             19
#define BSP_DO_EXT_0                  20
#define BSP_DO_EXT_1                  21  
#define BSP_DO_EXT_2                  22 
#define BSP_DO_EXT_3                  23 
#define BSP_DO_EXT_4                  24 
#define BSP_DO_EXT_5                  25
#define BSP_DO_MAX                    26
#endif




void bsp_do_init(void);
void bsp_do_low(int num);
void bsp_do_high(int num);

#endif