

#include "driver_flash.h"




driver_t *g_flash;

void flash_init(void)
{
  g_flash = driver_flash_open(FALSH_AT45DB);
}


int32_t flash_write(uint32_t offset, uint8_t* pData, uint32_t dataLen)
{
  return driver_flash_write(g_flash,  offset,  pData, dataLen);
}

void flash_read(uint32_t offset, uint8_t* pBuff,uint32_t buffSize, uint32_t readLen)
{
  driver_flash_read(g_flash,  offset, pBuff, buffSize,  readLen);
}