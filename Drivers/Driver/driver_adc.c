
#include "pcb_define.h"
#include "driver_adc.h"
#include "driver_stm32_adc.h"
#include "ads1220.h"
#include "util_memory.h"
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
  
}

float driver_adc_single_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err)
{
  const adc_api_t *api = drv->api;
  int32_t raw_now;

  float voltage;

  raw_now =  api->read_single(drv,channel,avg,err);

  if (strncmp(drv->name, "ADC_ADS1220",11)==0)
  {
    voltage = adc_driver_get_value(get_adc_config(0),ADC_CHANNEL_TYPE_SINGLE_ENDED, channel, raw_now);
  }
  else if (strncmp(drv->name, "STM32_ADC", 9) == 0)
  {
    voltage =
        adc_driver_get_value(get_adc_config(1),ADC_CHANNEL_TYPE_SINGLE_ENDED, channel, raw_now);
  }

  return voltage;
  
}

float driver_adc_diff_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err)
{
  const adc_api_t *api = drv->api;
  int32_t raw_now;

  float voltage;

  raw_now  = api->read_diff(drv, channel, avg, err);

  if (strncmp(drv->name, "ADC_ADS1220", 11) == 0)
  {
    voltage =
        adc_driver_get_value(get_adc_config(0), ADC_CHANNEL_TYPE_DIFFERENTIAL, channel, raw_now);
  }
  else if (strncmp(drv->name, "STM32_ADC", 9) == 0)
  {
    voltage =
        adc_driver_get_value(get_adc_config(1), ADC_CHANNEL_TYPE_DIFFERENTIAL, channel, raw_now);
  }

  return voltage;
}




int32_t driver_adc_single_raw_read(driver_t *drv, int channel, uint16_t avg, uint8_t *err)
{
  const adc_api_t *api = drv->api;

  return api->read_single(drv, channel, avg, err);


}

int32_t driver_adc_diff_raw_read(driver_t *drv, int channel, uint16_t avg, uint8_t *err)
{
  const adc_api_t *api = drv->api;

  return api->read_diff(drv, channel, avg, err);

}

void driver_adc_set(driver_t *drv, adc_set_option_t option, void *value)
{
  const adc_api_t *api = drv->api;

  switch (option)
  {
    case eADC_SET_OFFSET:
    {
      adc_offset_trim_t *p_offset = (adc_offset_trim_t*)value;
      adc_driver_adjust_offset_trim(get_adc_config(0), ADC_CHANNEL_TYPE_SINGLE_ENDED,
      p_offset->channel,p_offset->offset);
    }
    break;
    default:
      api->set(drv, option, value);
      break;
  }

}

bool driver_adc_get(driver_t *drv, adc_get_option_t option, void *para,void *value)
{
  const adc_api_t *api = drv->api;

  switch (option)
  {
    case eADC_GET_OFFSET:
    {
      float offset;
      adc_offset_trim_t *p_offset = (adc_offset_trim_t *)para;
      adc_driver_read_offset_trim(get_adc_config(0), p_offset->mode,
                                  p_offset->channel,&offset);
                                  p_offset->offset = offset;
    }
    break;
  }
}