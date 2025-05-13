
#ifndef DRIVER_DO_H
#define DRIVER_DO_H

#include "driver_interface.h"
#include "driver_do_define.h"

#define DO_PWR_CDMA       0
#define DO_ADC_NCS        1
#define DO_FRAM_CS        2
#define DO_RTC_CS         3
#define DO_FLASH_CS       4
#define DO_DIR_SDI        5
#define DO_DIR_RS485_A    6
#define DO_DIR_RS485_B    7
#define DO_EXT_0          8
#define DO_EXT_1          9
#define DO_EXT_2          10
#define DO_EXT_3          11
#define DO_EXT_4          12
#define DO_EXT_5          13
#define DO_HART_SEL       14
#define DO_HART_RTS       15
#define DO_POWER_HART_24V_ACTIVE_H 16     
#define DO_HART_RESET  17
#define DO_BTM_PWCTRL  18
#define DO_DIR_RS485_C 19
#define DO_DIR_RS485_D 20
#define DO_CON_PWR_RAIN_DECT_ACTIVE_H 21
#define DO_CON_PWR_RAIN_ACTIVE_H 22



driver_t *driver_do_open(uint32_t num,void *opt);
void driver_do_low(driver_t *drv);
void driver_do_high(driver_t *drv);


#endif