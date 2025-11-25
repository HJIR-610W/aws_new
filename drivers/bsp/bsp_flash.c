#include "bsp_flash.h"
#include "components\serial_flash\at45db.h"

void bsp_flash_init(void)
{
  at45db_init();
}
int32_t bsp_flash_write(uint32_t offset, uint8_t* pData, uint32_t dataLen)
{
  return at45db_write( offset, pData,  dataLen);
}
void bsp_flash_read(uint32_t offset, uint8_t* pBuff,  uint32_t readLen)
{
  at45db_read( offset,  pBuff,    readLen);
}