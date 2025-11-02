

#ifndef BSP_FLASH_H
#define BSP_FLASH_H

#include <stdint.h>

void bsp_flash_init(void);
int32_t bsp_flash_write(uint32_t offset, uint8_t* pData, uint32_t dataLen);
void bsp_flash_read(uint32_t offset, uint8_t* pBuff, uint32_t readLen);
#endif