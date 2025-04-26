
#include <stdint.h>

#include "app_sensor.h"
#include "driver_interface.h"

#include "general_adc.h"
#include "driver_adc.h"
#include "app_adc.h"



typedef struct general_adc_cfg_s
{
  driver_t *adc_io;
  int32_t highScale;
  int32_t lowScale;
  int32_t scale;
  int32_t outMaxVolt;
  int32_t outMinVolt;
  uint8_t channel;
  uint8_t mode;
}general_adc_cfg_t;


general_adc_cfg_t general_adc_cfg_single[16];
general_adc_cfg_t general_adc_cfg_diff[8];
driver_t general_adc_single[16];
driver_t general_adc_diff[8];






void *general_adc_open(uint8_t num,void *opt)
{
  adc_config_t *cfg = opt;


  if(cfg->mode ==0)//single
  {
    if(general_adc_single[cfg->channel].opened)
    {
      return &general_adc_single[cfg->channel];
    }  

    
    general_adc_cfg_single[cfg->channel].adc_io    = driver_adc_open(ADC_ADS1220,0);
    general_adc_cfg_single[cfg->channel].highScale = cfg->highScale;
    general_adc_cfg_single[cfg->channel].lowScale  = cfg->lowScale;
    general_adc_cfg_single[cfg->channel].scale     = cfg->scale;
    general_adc_cfg_single[cfg->channel].channel   = cfg->channel;
    general_adc_cfg_single[cfg->channel].mode      = cfg->mode;
    general_adc_cfg_single[cfg->channel].outMaxVolt      = cfg->outMaxV;
    general_adc_cfg_single[cfg->channel].outMinVolt      = cfg->outMinV;
    general_adc_single[cfg->channel].cfg = &general_adc_cfg_single[cfg->channel];
    general_adc_single[cfg->channel].name = "GENERAL_ADC";
    return &general_adc_single[cfg->channel];
  }
  else
  {
    if(general_adc_diff[cfg->channel].opened)
    {
      return &general_adc_diff[cfg->channel];
    }  
  
    general_adc_cfg_diff[cfg->channel].adc_io = driver_adc_open(ADC_ADS1220,0);
    general_adc_cfg_diff[cfg->channel].highScale = cfg->highScale;
    general_adc_cfg_diff[cfg->channel].lowScale  = cfg->lowScale;
    general_adc_cfg_diff[cfg->channel].scale     = cfg->scale;
    general_adc_cfg_diff[cfg->channel].channel = cfg->channel;
    general_adc_cfg_diff[cfg->channel].mode = cfg->mode;
    general_adc_cfg_diff[cfg->channel].outMaxVolt = cfg->outMaxV;
    general_adc_cfg_diff[cfg->channel].outMinVolt = cfg->outMinV;
    general_adc_diff[cfg->channel].name = "GENERAL_ADC";
    general_adc_diff[cfg->channel].cfg = &general_adc_cfg_diff[cfg->channel];
  
  return &general_adc_diff[cfg->channel];

  }

}



float general_adc_read(void *driver,uint8_t *err)
{
  general_adc_cfg_t *cfg = ((driver_t *)driver)->cfg;
  adc_config_t adc_config;
  
  adc_config.mode      = cfg->mode;
  adc_config.channel   = cfg->channel;
  adc_config.highScale = cfg->highScale;
  adc_config.lowScale  = cfg->lowScale;
  adc_config.scale     = cfg->scale;
  adc_config.outMaxV   = cfg->outMaxVolt;
  adc_config.outMinV   = cfg->outMinVolt;

  return cvt_voltateToData(&adc_config,err);
}

