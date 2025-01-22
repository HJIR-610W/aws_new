
#include "app_fram.h"
#include "config.h"
#include "driver_fram.h"







adc_cali_config_t g_adc_cali_config;
config_t config;
system_t System;


void config_factoryReset(void)
{
  
}


void check_config_limit(void)
{
  
}




void config_write_adcCalibraion(void)
{
  fram_write(0, (uint8_t *)&g_adc_cali_config, sizeof(g_adc_cali_config));
}

void config_write_config(void)
{
  fram_write(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config));
}


void config_write_s_config(void)
{
  fram_write(S_CONFIG_START_ADDRESS, (uint8_t *)&s_config, sizeof(s_config));
}


void config_init(void)
{
  fram_init();
  fram_read(ADC_CALI_START_ADDRESS, (uint8_t *)&g_adc_cali_config, sizeof(g_adc_cali_config));
  fram_read(S_CONFIG_START_ADDRESS, (uint8_t *)&s_config, sizeof(s_config));
  fram_read(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config));

}



