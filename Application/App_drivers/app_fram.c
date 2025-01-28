
#include "driver_fram.h"


driver_t *g_fram;


void fram_init(void)
{
  g_fram = driver_fram_open(FRAM_FM25LC);
}

void fram_read(uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
  driver_fram_read(g_fram, offset, pBuff, rLen);
}


void fram_write(uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
  driver_fram_write(g_fram, offset, pBuff, rLen);
}