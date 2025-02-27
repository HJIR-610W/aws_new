

#ifndef DRIVER_DI_H
#define DRIVER_DI_H

#include "driver_di_def.h"
#include "driver_interface.h"


#define DI_0_ADC_RDY     0
#define DI_1_RTC_IRQ     1
#define DI_RAIN_REED     2
#define DI_RAIN_HALL     3
#define DI_RAIN_HALL_ERR 4
#define DI_QUAD_UARTA_1  5
#define DI_QUAD_UARTB_2  6
#define DI_QUAD_UARTC_3  7
#define DI_QUAD_UARTD_4  8
#define DI_QUAD_UARTA_5  9
#define DI_QUAD_UARTB_6 10
#define DI_QUAD_UARTC_7 11
#define DI_QUAD_UARTD_8 12
#define DI_EXT_0        13
#define DI_EXT_1        14
#define DI_EXT_2        15
#define DI_EXT_3        16
#define DI_EXT_4        17
#define DI_EXT_5        18
#define DI_EXT_6        19
#define DI_EXT_7        20

#define DI_HART_CD      21
#define DI_USER_BTN     22

driver_t *driver_di_open(uint32_t num,void *opt);
void driver_di_close(driver_t *drv);
int32_t driver_di_read(driver_t *drv);
void driver_di_set(driver_t *drv,uint8_t cmd,void *option);

#endif