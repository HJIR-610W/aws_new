#ifndef CONFIG_NVM_H
#define CONFIG_NVM_H

#include "config_define.h"
#include "config_memory_map.h"

typedef struct sensor_nvm_S
{
  config_header_t header;
  uint8_t start;
  uint32_t yearRain;
  uint32_t yearSunshine;
  uint32_t monthRain;
  uint32_t monthSunshine;
  uint16_t logCnt;
} config_nvm_t;

void load_config_nvm(void);
void save_config_nvm(void);

#define WRITE_NVM(x)                                                          \
  fram_write(CONFIG_NVM_START_ADDRESS + (uint32_t)OFFSET_OF_STRUCT(config_nvm_t, x), \
             (uint8_t *)&g_config_nvm.x, sizeof(g_config_nvm.x));

             
extern config_nvm_t g_config_nvm;;

#endif