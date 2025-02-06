
#include "pcb_define.h"
#include "driver_adc.h"
#include "driver_stm32_adc.h"
#include "ads1220.h"
#include "utile.h"


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

int32_t driver_adc_single_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err)
{
  const adc_api_t *api = drv->api;

  return api->read_single(drv,channel,avg,err);
}

int32_t driver_adc_diff_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err)
{
  const adc_api_t *api = drv->api;

  return api->read_diff(drv,channel,avg,err);
}


void driver_adc_set(driver_t *drv, adc_set_option_t option, void *value)
{
  const adc_api_t *api = drv->api;


  api->set(drv,option,value);
}
