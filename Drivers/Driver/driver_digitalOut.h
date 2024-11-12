
#ifndef DRIVER_DIGITALOUT_H
#define DRIVER_DIGITALOUT_H

#include <stdint.h>

#include "cmsis_os.h"
#include "driver_interface.h"

#define DO_PWR_CDMA  0
#define DO_ADC_NCS   1
#define DO_FRAM_CS   2
#define DO_RTC_CS    3
#define DO_FLASH_CS  4

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


#define DO_NUM_MAX   15


driver_t * driver_do_open(uint32_t num);
void driver_do_low(driver_t *drv);
void driver_do_high(driver_t *drv);


#endif
