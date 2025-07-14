

#ifndef DRV_DO_H
#define DRV_DO_H


#define DRV_DO_EXT_0 17
#define DRV_DO_EXT_1 18
#define DRV_DO_EXT_2 19
#define DRV_DO_EXT_3 20
#define DRV_DO_EXT_4 21
#define DRV_DO_EXT_5 22

void drv_do_init(void);
void drv_do_low(int num);
void drv_do_high(int num);

#endif