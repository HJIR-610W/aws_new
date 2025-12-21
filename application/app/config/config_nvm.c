
#include <string.h>

#include "config_nvm.h"
#include "drv_fram.h"
#include "drv_crc.h"
#include "app_version.h"
#include "system_err.h"
config_nvm_t g_config_nvm;




void load_config_nvm(void)
{

#if 0
  uint8_t *p_start;
  uint32_t crc;

  drv_fram_read(CONFIG_START_ADDRESS, (uint8_t *)&g_config_nvm, sizeof(config_nvm_t));

  p_start = (uint8_t *)&g_config_nvm + sizeof(g_config_nvm.header);

  if (g_config_nvm.header.magicNum == CONFIG_MAGIC)
  {
    crc = drv_crc32_with_padding(p_start, sizeof(config_nvm_t) - sizeof(g_config_nvm.header));
    if (crc != g_config_nvm.header.crc)
    {

    }
  }

  #else

  drv_fram_read(CONFIG_NVM_START_ADDRESS, (uint8_t *)&g_config_nvm, sizeof(config_nvm_t));

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"sizeof(config_nvm_t):%d",sizeof(config_nvm_t));
#endif



}

void save_config_nvm(void)
{
  uint32_t crc;
  uint8_t *p_start;

  p_start = (uint8_t *)&g_config_nvm + sizeof(g_config_nvm.header);

  crc = drv_crc32_with_padding(p_start,sizeof(config_nvm_t) - sizeof(g_config_nvm.header));

  g_config_nvm.header.time_stamp = 0;
  g_config_nvm.header.magicNum = CONFIG_MAGIC;
  g_config_nvm.header.crc = crc;
  g_config_nvm.header.version = CONFIG_NVM_VERSION;

  drv_fram_write(CONFIG_NVM_START_ADDRESS, (uint8_t *)&g_config_nvm, sizeof(config_nvm_t));

}

config_nvm_t *get_config_nvm(void)
{
    return &g_config_nvm;
}


void nvm_set_log_cnt(uint32_t value)
{
  g_config_nvm.log_q_cnt = value;
 save_config_nvm();
 
}

uint32_t nvm_get_log_cnt(void)
{ 
  return g_config_nvm.log_q_cnt; 
};

void nvm_set_alarm_count(uint32_t value)
{
  g_config_nvm.log_alarm_count = value;
 save_config_nvm();
 
}

uint32_t nvm_get_alarm_count(void)
{ 
  return g_config_nvm.log_alarm_count; 
};