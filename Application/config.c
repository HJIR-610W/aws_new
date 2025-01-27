
#include "app_fram.h"
#include "config.h"
#include "driver_fram.h"

#include "app_rs232.h"
#include "app_rs485.h"





adc_cali_config_t g_adc_cali_config;
config_t config;
system_t System;


void config_factoryReset(void)
{
  
}


void check_config_limit(void)
{

  for(int i = 0 ; i < _countof(s_config.rs232);i++)
  {
    if(s_config.rs232[i].port >= eRS232_MAX)
    {
      s_config.rs232[i].port = 0;
       WRITE_S_CFG(rs232[i].port);
    }

    if(s_config.rs232[i].parityIdx >= 2)
    {
      s_config.rs232[i].parityIdx = 0;
      WRITE_S_CFG(rs232[i].parityIdx);
    }
  }


#if 0 
  for(int i = 0 ; i < _countof(config.sensor);i++)
  {
    int row;
    int cnt;
    cnt = config.sensor[i].configCnt;
    row = config.sensor[i].config[0][0];
    for(int j=i+1;j< _countof(config.sensor); j++)
    {
      for(int n = 0; n< 4; n++)
      {
        if(config.sensor[i].config[i][0]==config.sensor[j].config[n][0])
        {
          if(config.sensor[i].config[i][1]==config.sensor[j].config[n][1])
          {
            config.sensor[j].configCnt = 0;
          }
        }
      }
    }

  }
#endif

}




void config_write_adcCalibraion(void)
{
  fram_write(0, (uint8_t *)&g_adc_cali_config, sizeof(g_adc_cali_config));
}

/**
 * @brief config 저장
 */
void write_config(void)
{
  fram_write(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config));
}

/**
 * @brief sensor config 저장
 */
void write_s_config(void)
{
  fram_write(S_CONFIG_START_ADDRESS, (uint8_t *)&s_config, sizeof(s_config));
}


void config_init(void)
{
  fram_init();
  
  fram_read(ADC_CALI_START_ADDRESS, (uint8_t *)&g_adc_cali_config, sizeof(g_adc_cali_config));
  fram_read(S_CONFIG_START_ADDRESS, (uint8_t *)&s_config, sizeof(s_config));
  fram_read(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config));

  check_config_limit();
}



float get_scale(uint8_t sensor)
{
  return config.sensor[sensor].scale;
}