
#include "config_nvm.h"
#include "drv_fram.h"
#include "drv_crc.h"
#include "app_version.h"

config_nvm_t g_config_nvm;

void load_config_nvm(void)
{
  drv_fram_read(CONFIG_NVM_START_ADDRESS, (uint8_t *)&g_config_nvm, sizeof(g_config_nvm));
}

void save_config_nvm(void)
{
    uint32_t crc;


    g_config_nvm.start = 0;
    crc = drv_crc32_with_padding(&g_config_nvm.start,
                                sizeof(config_nvm_t) - sizeof(g_config_nvm.header));

    g_config_nvm.header.magicNum = CONFIG_MAGIC;
    g_config_nvm.header.crc = crc;
    g_config_nvm.header.version = get_app_version(0,0,0,0);

    drv_fram_write(CONFIG_NVM_START_ADDRESS, (uint8_t *)&g_config_nvm, sizeof(g_config_nvm));
}

config_nvm_t *get_config_nvm(void)
{
    return &g_config_nvm;
}


void nvm_set_log_cnt(uint32_t value)
{
  g_config_nvm.log_q_cnt = value;
  WRITE_NVM(log_q_cnt);
}

uint32_t nvm_get_log_cnt(void) { return g_config_nvm.log_q_cnt; };