

#ifndef DRV_POWER_H
#define DRV_POWER_H

#define DRV_POWER_CDMA 0
#define DRV_POWER_HART_24V 1
#define RV_POWER_LCD_RESET 2
#define DO_POWER_RAIN_DECT_ANALOG 3
#define DO_POWER_RAIN_DECT_DIGITAL 4



void drv_power_init(void);
void drv_power_on(int num);
void drv_power_off(int num);


#endif