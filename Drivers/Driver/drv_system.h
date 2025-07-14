
#ifndef DRV_SYSTEM_H
#define DRV_SYSTEM_H

#define DRV_SYS_BATTERY      0
#define DRV_SYS_TEMPERATURE  1

void drv_system_init(void);
float drv_system_read(int num);

#endif