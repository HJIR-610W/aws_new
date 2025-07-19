
#ifndef BSP_FRAM_H
#define BSP_FRAM_H
#include <stdint.h>

void bsp_fram_init(void);
void bsp_fram_read(uint32_t offset, unsigned char* pBuff, uint16_t rLen);

void bsp_fram_write(uint32_t offset, unsigned char* pBuff, uint16_t rLen);

#endif