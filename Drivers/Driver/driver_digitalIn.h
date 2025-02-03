
#ifndef DRIVER_DIGITALIN_H
#define DRIVER_DIGITALIN_H


#include <stdint.h>

#include "pcb_define.h"

#include "cmsis_os.h"
#include "driver_interface.h"
#include "driver_di_def.h"


#define DI_ADC_RDY       0
#define DI_RTC_IRQ       1
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


#define DI_MAX          13


#define DI_SET_ISR 0





driver_t *driver_di_open(uint32_t num);
int32_t driver_di_read(driver_t *drv);
void driver_di_set(driver_t *drv,uint8_t cmd,void *option);

#endif
