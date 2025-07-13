
#ifndef DRV_CRC_H
#define DRV_CRC_H

#include <stdint.h>
#include <stddef.h>

void drv_crc_init(void);
uint32_t drv_crc32_with_padding(const uint8_t *data, size_t len);

#endif