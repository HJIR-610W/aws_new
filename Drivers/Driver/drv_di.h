

#ifndef DRV_DI_H
#define DRV_DI_H

#include <stdint.h>
#include "driver_di_def.h"



#define DRV_DI_USER_BTN 3
#define DRV_DI_RAIN_REED 4
#define DRV_DI_RAIN_HALL 5
#define DRV_DI_RAIN_HALL_ERR 6
#define DRV_DI_RAIN_DETECT 7


#define DRV_DI_0 16
#define DRV_DI_1 17
#define DRV_DI_2 18
#define DRV_DI_3 19
#define DRV_DI_4 20
#define DRV_DI_5 21
#define DRV_DI_6 22
#define DRV_DI_7 23
#define DRV_DI_8 24


void drv_di_init(void);
int32_t drv_di_read(int di_number);

void drv_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg);

#endif