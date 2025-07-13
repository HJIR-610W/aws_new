

#ifndef DRV_DO_H
#define DRV_DO_H

#define DRV_DO_EXT_0 0  // App 정의되지 않음
#define DRV_DO_EXT_1 1  // App 정의되지 않음
#define DRV_DO_EXT_2 2  // App 정의되지 않음
#define DRV_DO_EXT_3 3  // App 정의되지 않음
#define DRV_DO_EXT_4 4  // App 정의되지 않음
#define DRV_DO_EXT_5 5  // App 정의되지 않음


void drv_do_init(void);
void drv_do_low(int num);
void drv_do_high(int num);

#endif