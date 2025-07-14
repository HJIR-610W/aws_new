#include "components\fram\fm25cl.h"


void bsp_fram_init(void)
{
   fm25lc_init();
}
void bsp_fram_read(uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
  fm25cl_read( offset, pBuff,  rLen);
}
void bsp_fram_write(uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
  fm25cl_write(offset, pBuff, rLen);
}
