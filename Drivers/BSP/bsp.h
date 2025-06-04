

#ifndef BSP_H_
#define BSP_H_

#include <stdbool.h>

#include "bsp_do.h"
#include "bsp_di.h"
#include "bsp_rtc.h"

void bsp_init(void);

void bsp_cdma_power_on(void);
void bsp_cdma_power_off(void);


float bsp_read_battery(void);    // 보드 전원
float bsp_read_temperature(void);// 보드 온도

bool bsp_door_opened(void);
#endif