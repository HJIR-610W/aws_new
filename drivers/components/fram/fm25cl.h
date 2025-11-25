
#ifndef FM25CL_H
#define FM25CL_H

#include <stdint.h>





void fm25lc_init(void);
void fm25cl_write(uint32_t offset,const uint8_t *pData,uint16_t wLen);
void fm25cl_read(uint32_t offset,uint8_t *pBuff,uint16_t rLen);

int fm25cl_lfs_read(uint32_t block, uint32_t off, uint8_t *buffer, uint32_t size);
int fm25cl_lfs_prog( uint32_t block,uint32_t off,const uint8_t *buffer,uint32_t size);
#endif
