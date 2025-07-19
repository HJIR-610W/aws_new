
#include "drv_fram.h"
#include "bsp_fram.h"

void drv_fram_init(void)
{
   bsp_fram_init();
}
void drv_fram_read(uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
  bsp_fram_read( offset, pBuff,  rLen);

}
void drv_fram_write(uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
  bsp_fram_write( offset, pBuff, rLen);
}