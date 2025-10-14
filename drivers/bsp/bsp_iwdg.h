

#ifndef BSP_IWDG_H
#define BSP_IWDG_H
#include <stdint.h>

void bsp_iwdg_init(uint32_t timeout_ms);
void bsp_iwdg_reload(void);
#endif