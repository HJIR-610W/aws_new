
#ifndef DRIVER_DO_H
#define DRIVER_DO_H

#include "driver_interface.h"
#include "driver_do_define.h"

#define DO_PWR_CDMA        0
#define DO_ADC_NCS         1
#define DO_FRAM_CS         2
#define DO_RTC_CS          3
#define DO_FLASH_CS        4
#define DO_CON_PWR_232_A   5
#define DO_CON_PWR_232_B   6
#define DO_CON_PWR_485     7
#define DO_CON_PWR_TC      8
#define DO_CON_PWR_DSEN    9
#define DO_CON_PWR_ASEN   10
#define DO_CON_PWR_ASEN_A 11
#define DO_CON_PWR_ASEN_B 12
#define DO_CON_PWR_ASEN_C 13
#define DO_CON_PWR_ASEN_D 14
#define DO_DIR_SDI        15
#define DO_DIR_RS485_A    16
#define DO_DIR_RS485_B    17
#define DO_EXT_0          18
#define DO_EXT_1          19
#define DO_EXT_2          20
#define DO_EXT_3          21
#define DO_EXT_4          22
#define DO_EXT_5          23
#define DO_EXT_6          24
#define DO_EXT_7          25

#define DO_HART_SEL       26
#define DO_HART_RTS       27

#define DO_POWER_24V_ACTIVE_H 28     
#define DO_HART_RESET 29


driver_t *driver_do_open(uint32_t num,void *opt);
void driver_do_low(driver_t *drv);
void driver_do_high(driver_t *drv);


#endif