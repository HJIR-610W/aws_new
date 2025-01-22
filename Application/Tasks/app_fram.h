

#ifndef APP_FRAM_H
#define APP_FRAM_H

#include <stdint.h>

void fram_init(void);
void fram_read(uint32_t offset, unsigned char* pBuff, uint16_t rLen);
void fram_write(uint32_t offset, unsigned char* pBuff, uint16_t rLen);
#endif