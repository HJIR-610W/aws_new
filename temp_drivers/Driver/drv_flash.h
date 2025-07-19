

#ifndef DRV_FLASH_H
#define DRV_FLASH_H

#include <stdint.h>

void drv_flash_init(void);
int32_t drv_flash_write(uint32_t offset, uint8_t* pData, uint32_t dataLen);
void drv_flash_read(uint32_t offset, uint8_t* pBuff, uint32_t buffSize, uint32_t readLen);

#endif