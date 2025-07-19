#include "bsp_flash.h"

void drv_flash_init(void)
{
  bsp_flash_init();
}
int32_t drv_flash_write(uint32_t offset, uint8_t* pData, uint32_t dataLen)
{
  return bsp_flash_write( offset,  pData, dataLen);
}
void drv_flash_read(uint32_t offset, uint8_t* pBuff, uint32_t buffSize, uint32_t readLen)
{
  bsp_flash_read( offset,  pBuff, buffSize, readLen);
}