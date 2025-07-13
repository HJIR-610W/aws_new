

#ifndef DRV_DI_H
#define DRV_DI_H

#include <stdint.h>
#include "driver_di_def.h"

#define DRV_DI_0 0
#define DRV_DI_1 1
#define DRV_DI_2 2
#define DRV_DI_3 3
#define DRV_DI_4 4
#define DRV_DI_5 5
#define DRV_DI_6 6
#define DRV_DI_7 7

#define DI_RAIN_REED      8    
#define DI_RAIN_HALL      9     
#define DI_RAIN_HALL_ERR 10  
#define DI_USER_BTN      11    
#define DI_RAIN_DETECT   12  

void drv_di_init(void);
int32_t drv_di_read(int di_number);

void drv_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg);

#endif