
#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif



void bsp_crc_init(void);
uint32_t bsp_crc32_hw_with_padding(const uint8_t *data, size_t len);




#ifdef __cplusplus
}
#endif

#endif

