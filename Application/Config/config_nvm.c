
#include "config_nvm.h"
#include "app_fram.h"
#include "crc.h"
#include "app_version.h"

config_nvm_t g_config_nvm;

void load_config_nvm(void)
{
    fram_read(CONFIG_NVM_START_ADDRESS, (uint8_t *)&g_config_nvm, sizeof(g_config_nvm));
}

void save_config_nvm(void)
{
    uint32_t crc;
    uint8_t temp;

    g_config_nvm.start = 0;
    crc = crc32_hw_with_padding(&g_config_nvm.start,
                                sizeof(config_nvm_t) - sizeof(g_config_nvm.header));

    g_config_nvm.header.magicNum = CONFIG_MAGIC;
    g_config_nvm.header.crc = crc;
    g_config_nvm.header.version = get_app_version();

    fram_write(CONFIG_NVM_START_ADDRESS, (uint8_t *)&g_config_nvm, sizeof(g_config_nvm));

}

config_nvm_t *get_config_nvm(void)
{
    return &g_config_nvm;
}



void nvm_set_rainfall_yearly(float value)
{
  g_config_nvm.rainfall_yearly = value;

  WRITE_NVM(rainfall_yearly);
}

void nvm_set_rainfall_monthly(float value)
{
  g_config_nvm.rainfall_monthly = value;
  WRITE_NVM(rainfall_monthly);
}

void nvm_set_sunshine_yearly(uint32_t value)
{
  g_config_nvm.sunshine_yearly = value;
  WRITE_NVM(sunshine_yearly);
}

void nvm_set_sunshine_monthly(uint32_t value)
{
  g_config_nvm.sunshine_monthly = value;
  WRITE_NVM(sunshine_monthly);
}

void nvm_set_log_cnt(uint16_t value)
{
  g_config_nvm.logCnt = value;
  WRITE_NVM(logCnt);
}
