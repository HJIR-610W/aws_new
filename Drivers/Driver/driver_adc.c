
#include "pcb_define.h"
#include "driver_adc.h"
#include "driver_stm32_adc.h"
#include "ads1220.h"
#include "utile.h"
#include "adc_calibration.h"

driver_t *driver_adc_open(uint32_t num,void *opt)
{
  driver_t *driver = NULL;

  switch(num)
  {
    case ADC_ADS1220:
    driver = ads1220_open(ADC_ADS1220,opt);
    break;
    case ADC_STM32:
    driver = driver_stm32_adc_open(ADC_STM32,opt);
    break;
  }

  return driver;
}

void driver_close(driver_t *drv)
{
  const adc_api_t *api = drv->api;
}

float driver_adc_single_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err)
{
  const adc_api_t *api = drv->api;
  int32_t raw_now;
  int32_t channel_calculated=0;
  float voltage;

  raw_now =  api->read_single(drv,channel,avg,err);

  if (strncmp(drv->name, "ADC_ADS1220",11)==0)
  {
    channel_calculated = channel;
  }
  else if (strncmp(drv->name, "STM32_ADC", 9) == 0)
  {
    channel_calculated = ADC_ADS1220_S_CH_17+ 1 + channel;
  }

  //켈리브레이션은 하나로 관리하다보니 0~17은 ads, 18~19 stm32
  voltage = adc_driver_get_value(ADC_CHANNEL_TYPE_SINGLE_ENDED, channel_calculated, raw_now);

  return voltage;
  
}

float driver_adc_diff_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err)
{
  const adc_api_t *api = drv->api;
  int32_t raw_now;
  int32_t channel_calculated = 0;
  float voltage;

  raw_now  = api->read_diff(drv, channel, avg, err);

  // 켈리브레이션은 하나로 관리하다보니 0~17은 ads, 18~19 stm32
  voltage = adc_driver_get_value(ADC_CHANNEL_TYPE_DIFFERENTIAL, channel_calculated, raw_now);

  return voltage;
}


void driver_adc_set(driver_t *drv, adc_set_option_t option, void *value)
{
  const adc_api_t *api = drv->api;


  api->set(drv,option,value);
}



