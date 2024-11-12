

#include "config.h"


adc_config_t g_adc_config;


void adcSingleChCfg_set(uint8_t singleChannel,bool enable)
{
  uint8_t diffChNum;

  diffChNum = singleChannel/4;

  if(enable)
  {
    g_adc_config.singleChEn[singleChannel] = ENABLE;
    g_adc_config.diffChEn_1[diffChNum] = DISABLE;
  }
  else
  {
    g_adc_config.singleChEn[singleChannel] = DISABLE;

    if(singleChannel%2)// 홀수번 채널이면
    {
      if(g_adc_config.singleChEn[singleChannel-1]==DISABLE)//짝수번 채널도 disable이면
      {
            g_adc_config.diffChEn_1[diffChNum] = ENABLE;// 차동채널 사용 가능
      }
    }
   else
   {
      if(g_adc_config.singleChEn[singleChannel+1]==DISABLE)//홀수번 채널도 disable이면
      {
            g_adc_config.diffChEn_1[diffChNum] = ENABLE;// 차동채널 사용 가능
      }
   }
  }
}




