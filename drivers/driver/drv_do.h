

#ifndef DRV_DO_H
#define DRV_DO_H
#include "bsp_do.h"



#define DRV_DO_EXT_0 BSP_DO_EXT_0
#define DRV_DO_EXT_1 BSP_DO_EXT_1
#define DRV_DO_EXT_2 BSP_DO_EXT_2
#define DRV_DO_EXT_3 BSP_DO_EXT_3
#define DRV_DO_EXT_4 BSP_DO_EXT_4
#define DRV_DO_EXT_5 BSP_DO_EXT_5

void drv_do_init(void);
void drv_do_low(int num);
void drv_do_high(int num);

#endif