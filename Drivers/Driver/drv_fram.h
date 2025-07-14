
#ifndef DRV_FRAM_H
#define DRV_FRAM_H

#include <stdint.h>

void drv_fram_init(void);
void drv_fram_read(uint32_t offset, unsigned char* pBuff, uint16_t rLen);
void drv_fram_write( uint32_t offset, unsigned char* pBuff, uint16_t rLen);

#endif