

#ifndef DRIVER_DI_H
#define DRIVER_DI_H

#include "driver_di_def.h"
#include "driver_interface.h"

#define DI_EXT_0 100         // App door
#define DI_EXT_1 101         // App 정의되지 않음
#define DI_EXT_2 102         // App 정의되지 않음
#define DI_EXT_3 103         // App 정의되지 않음
#define DI_EXT_4 105         // App 정의되지 않음
#define DI_EXT_5 106         // App 정의되지 않음

#define DI_0_ADC_RDY     0  // Driver
#define DI_1_RTC_IRQ     1  // Driver
#define DI_RAIN_REED     2  // Driver
#define DI_RAIN_HALL     3  // Driver
#define DI_RAIN_HALL_ERR 4  // Driver
#define DI_QUAD_UARTA_1  5  // Driver INT
#define DI_QUAD_UARTB_2  6  // Driver INT
#define DI_QUAD_UARTC_3  7  // Driver INT
#define DI_QUAD_UARTD_4  8  // Driver INT
#define DI_QUAD_UARTA_5  9  // Driver INT
#define DI_QUAD_UARTB_6 10  // Driver INT
#define DI_QUAD_UARTC_7 11  // Driver INT
#define DI_QUAD_UARTD_8 12  // Driver INT
#define DI_HART_CD      13  // Driver
#define DI_USER_BTN     14  // Driver
#define DI_BTM_STATUS   15  // Driver
#define DI_RAIN_DETECT  16  // Driver

driver_t *driver_di_open(uint32_t num,void *opt);
void driver_di_close(driver_t *drv);
int32_t driver_di_read(driver_t *drv);
void driver_di_set(driver_t *drv, di_set_option_t cmd, void *option);
bool driver_di_is_low(driver_t *di, uint32_t hold_time_ms, uint32_t debounce_ms);

#endif