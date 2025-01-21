

#include "config.h"
#include "driver_fram.h"



driver_t *g_framCfg;
adc_cali_config_t g_adc_cali_config;










temp_config_t g_temp_config;

config_t config;

adc_config_t g_adc_config;


adc_cfg_t g_adc_cfg[50];




void adcSingleChCfg_set(uint8_t singleChannel,bool enable)
{
  uint8_t diffChNum;

  diffChNum = singleChannel/4;

  if(enable)
  {
    g_adc_config.singleChEn[singleChannel] = USER_ENABLE;
    g_adc_config.diffChEn_1[diffChNum] = USER_DISABLE;
  }
  else
  {
    g_adc_config.singleChEn[singleChannel] = USER_DISABLE;

    if(singleChannel%2)// 홀수번 채널이면
    {
      if(g_adc_config.singleChEn[singleChannel-1]==USER_DISABLE)//짝수번 채널도 disable이면
      {
            g_adc_config.diffChEn_1[diffChNum] = USER_ENABLE;// 차동채널 사용 가능
      }
    }
   else
   {
      if(g_adc_config.singleChEn[singleChannel+1]==USER_DISABLE)//홀수번 채널도 disable이면
      {
            g_adc_config.diffChEn_1[diffChNum] = USER_ENABLE;// 차동채널 사용 가능
      }
   }
  }
}





#define FRAM_FM25LC       0






void config_init(void)
{
  g_framCfg = driver_fram_open(FRAM_FM25LC);
  
  
  
  driver_fram_read(g_framCfg, 0, (uint8_t *)&g_adc_cali_config, sizeof(g_adc_cali_config));


  
  

}


void config_write_adcCalibraion(void)
{
  driver_fram_write(g_framCfg, 0, (uint8_t *)&g_adc_cali_config, sizeof(g_adc_cali_config));

  
}

void config_factoryReset(void)
{
  
}


void check_config_limit(void)
{
  
}