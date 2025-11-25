#include "at45db.h"

void drv_flash_init(void)
{
  at45db_init();
}
int32_t drv_flash_write(uint32_t offset, uint8_t* pData, uint32_t dataLen)
{
  //return at45db_write_fast( offset,  pData, dataLen);
    return at45db_write( offset,  pData, dataLen);
}
void drv_flash_read(uint32_t offset, uint8_t* pBuff,  uint32_t readLen)
{
  //at45db_read_fast( offset,  pBuff,  readLen);
  at45db_read( offset,  pBuff,  readLen);
}