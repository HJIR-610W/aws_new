#ifndef CONFIG_NVM_H
#define CONFIG_NVM_H

#include "config_define.h"
#include "config_memory_map.h"
#include "util_memory.h"
typedef struct sensor_nvm_S
{
  config_header_t header;
  uint8_t start;
  uint32_t log_q_cnt;
} config_nvm_t;

void load_config_nvm(void);
void save_config_nvm(void);

#define WRITE_NVM(x)                                                                     \
  drv_fram_write(CONFIG_NVM_START_ADDRESS + (uint32_t)OFFSET_OF_STRUCT(config_nvm_t, x), \
                 (uint8_t *)&g_config_nvm.x, sizeof(g_config_nvm.x));

config_nvm_t *get_config_nvm(void);
void nvm_set_log_cnt(uint32_t value);
uint32_t nvm_get_log_cnt(void);

#endif