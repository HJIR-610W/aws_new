#include "drv_crc.h"
#include "bsp_crc.h"

uint32_t drv_crc32_with_padding(const uint8_t *data, size_t len)
{
  return bsp_crc32_hw_with_padding(data,len);
}