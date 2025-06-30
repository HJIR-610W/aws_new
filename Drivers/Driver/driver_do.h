
#ifndef DRIVER_DO_H
#define DRIVER_DO_H

#include "driver_interface.h"
#include "driver_do_define.h"

#define DO_EXT_0 100  //App 정의되지 않음
#define DO_EXT_1 101  // App 정의되지 않음
#define DO_EXT_2 102  // App 정의되지 않음
#define DO_EXT_3 103  // App 정의되지 않음
#define DO_EXT_4 104  // App 정의되지 않음
#define DO_EXT_5 105  // App 정의되지 않음

#define DO_POWER_CDMA      0  // BSP
#define DO_POWER_HART_24V  1  // BSP
#define DO_BTM_PWCTRL      2  // BSP
#define DO_POWER_RAIN_DECT_DIGITAL 3  // BSP
#define DO_POWER_RAIN_DECT_ANALOG 4  // BSP J37.1

#define DO_ADC_NCS        5  // Driver
#define DO_FRAM_CS        6    // Driver
#define DO_RTC_CS         7    // Driver
#define DO_FLASH_CS       8    // Driver
#define DO_DIR_SDI        9    // Driver
#define DO_DIR_RS485_A    10    // Driver
#define DO_DIR_RS485_B    11    // Driver
#define DO_HART_SEL      12       // BSP
#define DO_HART_RTS      13       // Driver
#define DO_HART_RESET    14
#define DO_DIR_RS485_C   15
#define DO_DIR_RS485_D   16





driver_t *driver_do_open(uint32_t num,void *opt);
void driver_do_low(driver_t *drv);
void driver_do_high(driver_t *drv);


#endif