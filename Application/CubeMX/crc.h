
#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "pcb_define.h"



extern uint32_t crc32_hw_with_padding(const uint8_t *data, size_t len);



void MX_CRC_Init(void);



#ifdef __cplusplus
}
#endif

#endif

