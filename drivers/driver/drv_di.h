

#ifndef DRV_DI_H
#define DRV_DI_H

#include <stdint.h>
#include "drv_di_def.h"
#include "bsp_di.h"


#define DRV_DI_USER_BTN      BSP_DI_USER_BTN
#define DRV_DI_RAIN_REED     BSP_DI_RAIN_REED
#define DRV_DI_RAIN_HALL     BSP_DI_RAIN_HALL
#define DRV_DI_RAIN_HALL_ERR BSP_DI_RAIN_HALL_ERR
#define DRV_DI_DI_RAIN_DETECT_ANALOG   BSP_DI_RAIN_DETECT_A
#define DRV_DI_0 BSP_DI_0
#define DRV_DI_1 BSP_DI_1
#define DRV_DI_2 BSP_DI_2
#define DRV_DI_3 BSP_DI_3
#define DRV_DI_4 BSP_DI_4
#define DRV_DI_RAIN_DETECT_DIGITAL BSP_DI_5

void drv_di_init(void);
int32_t drv_di_read(int di_number);

void drv_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg);

#endif