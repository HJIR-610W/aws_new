
#ifndef CONFIG_DEFINE_H

#define CONFIG_DEFINE_H

#include <stdint.h>

#define CONFIG_MAGIC 0x5a5a5a5a

// 4바이트 정렬 필수
typedef struct config_header_s
{
  uint32_t magicNum;
  uint32_t version;
  uint32_t crc;
} config_header_t;



#endif