
#ifndef APP_FLASH_H
#define APP_FLASH_H

#include <stdint.h>

#define LOG_START_ADDRESS  0x00000000
#define DATA_START_ADDRESS 0x00019000


void flash_init(void);
int32_t flash_write(uint32_t offset, uint8_t* pData, uint32_t dataLen);
void flash_read(uint32_t offset, uint8_t* pBuff,uint32_t buffSize, uint32_t readLen);

#endif