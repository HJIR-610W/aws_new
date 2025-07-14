

#ifndef BSP_POWER_H
#define BSP_POWER_H

#define BSP_POWER_CDMA 0
#define BSP_POWER_HART_24V 1
#define BSP_POWER_LCD_RESET 2
#define BSP_POWER_RAIN_DECT_ANALOG 3
#define BSP_POWER_RAIN_DECT_DIGITAL 4


void bsp_power_init(void);
void bsp_power_on(int num);
void bsp_power_off(int num);


#endif