
#ifndef BSP_DO_H
#define BSP_DO_H


#define BSP_DO_POWER_CDMA 0             
#define BSP_DO_POWER_HART_24V 1         
#define BSP_DO_LCD_RESET 2               
#define BSP_DO_POWER_RAIN_DECT_DIGITAL 3  
#define BSP_DO_POWER_RAIN_DECT_ANALOG 4  
#define BSP_DO_ADC_NCS 5       
#define BSP_DO_FRAM_CS 6       
#define BSP_DO_RTC_CS 7        
#define BSP_DO_FLASH_CS 8      
#define BSP_DO_DIR_SDI 9      
#define BSP_DO_DIR_RS485_A 10  
#define BSP_DO_DIR_RS485_B 11 
#define BSP_DO_HART_SEL 12     
#define BSP_DO_HART_RTS 13    
#define BSP_DO_HART_RESET 14
#define BSP_DO_DIR_RS485_C 15
#define BSP_DO_DIR_RS485_D 16

#define BSP_DO_EXT_0 17 
#define BSP_DO_EXT_1 18  
#define BSP_DO_EXT_2 19 
#define BSP_DO_EXT_3 20 
#define BSP_DO_EXT_4 21 
#define BSP_DO_EXT_5 22

#define BSP_DO_MAX 23
void bsp_do_init(void);
void bsp_do_low(int num);
void bsp_do_high(int num);

#endif