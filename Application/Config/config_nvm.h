#ifndef CONFIG_NVM_H
#define CONFIG_NVM_H

#include "config_define.h"
#include "config_memory_map.h"
#include "utile.h"
typedef struct sensor_nvm_S
{
  config_header_t header;
  uint8_t start;
  float rainfall_yearly;
  float rainfall_monthly;
  uint32_t sunshine_yearly;
  uint32_t sunshine_monthly;
  uint16_t logCnt;
} config_nvm_t;

void load_config_nvm(void);
void save_config_nvm(void);

#define WRITE_NVM(x)                                                          \
  fram_write(CONFIG_NVM_START_ADDRESS + (uint32_t)OFFSET_OF_STRUCT(config_nvm_t, x), \
             (uint8_t *)&g_config_nvm.x, sizeof(g_config_nvm.x));


void nvm_set_rainfall_yearly(float value);
void nvm_set_rainfall_monthly(float value);
void nvm_set_sunshine_yearly(uint32_t value);
void nvm_set_sunshine_monthly(uint32_t value);
void set_log_cnt(uint16_t value);


extern config_nvm_t g_config_nvm;;

config_nvm_t *get_config_nvm(void);

#endif