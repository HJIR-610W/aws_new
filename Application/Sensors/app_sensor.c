
#include "app_sensor.h"
#include "utile.h"
config_manager_t s_config;




adc_config_t * get_adc_config(sensor_t *sensor)
{
  for (int i = 0; i < sensor->configCnt; i++)
  {
    if (sensor->config[i][0] == S_T_ADC)
    {
      return &s_config.adc[sensor->config[i][1]];
    }
  }
  return 0;
}

void set_adc_config(sensor_t *sensor,adc_config_t *adc)
{
  for (int i = 0; i < sensor->configCnt; i++)
  {
    if (sensor->config[i][0] == S_T_ADC)
    {
      s_config.adc[sensor->config[i][1]] = *adc;
      break;
    }
  }
}


void set_sensor_type(sensor_t *sensor,uint8_t type)
{
  sensor->type = type;
}




/**
 * @brief 센서에 ADC 설정값 추가
 * 
 */
void add_adc_sensor_config(sensor_t *sensor,adc_config_t *adc)
{
  uint8_t cnt=0;

  for(int i = 0 ; i< _countof(sensor->config);i++)
  {
    if(sensor->config[i][0] == S_T_ADC)
    {
      s_config.adc[sensor->config[i][1]] = *adc;
    }
  }

  if(cnt==0)
  {
    sensor->config[sensor->configCnt++][0] = S_T_ADC;
    s_config.adc[s_config.adc_cnt++] = *adc;
  }
}



