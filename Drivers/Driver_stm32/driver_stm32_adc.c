

#include "pcb_define.h"
#include "driver_stm32_adc.h"

typedef struct
{
  ADC_HandleTypeDef handle;
}stm32_adc_config_t;


driver_t stm32_adc_driver;
stm32_adc_config_t stm32_adc_config;


driver_t *driver_stm32_adc_open(uint32_t num,void *opt)
{
  if(stm32_adc_driver.opened)
  {
    return &stm32_adc_driver;
  }

  stm32_adc_driver.opened = true;
  stm32_adc_driver.cfg = &stm32_adc_config;

}



void stm32_adc_close(driver_t *handle)
{

}
int32_t stm32_adc_read_single(driver_t *handle,int channel,uint16_t avg,uint8_t *err)
{

}

int32_t stm32_adc_read_diff(driver_t *handle,int channel,uint16_t avg,uint8_t *err)
{

}

void stm32_adc_set(driver_t *handle, adc_set_option_t option, void *value)
{

}