
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
  uint8_t channel;
}general_adc_cfg_t;


general_adc_cfg_t general_adc_cfg_single[16];
general_adc_cfg_t general_adc_cfg_diff[8];
driver_t general_adc_single[16];
driver_t general_adc_diff[8];

void *general_adc_single_open(uint8_t num,void *opt)
{
  adc_config_t *cfg = opt;

  if(general_adc_single[num].opened)
  {
    return &general_adc_single[num];
  }  

  general_adc_cfg_single[num].adc_io = driver_adc_open(ADC_ADS1220,0);
  general_adc_cfg_single[num].highScale = cfg->highScale;
  general_adc_cfg_single[num].lowScale  = cfg->lowScale;
  general_adc_cfg_single[num].scale     = cfg->scale;

  general_adc_single[num].cfg = &general_adc_cfg_single[num];

return &general_adc_single[num];
}

void *general_adc_diff_open(uint8_t num,void *opt)
{
  adc_config_t *cfg = opt;
  
  if(general_adc_single[num].opened)
  {
    return &general_adc_single[num];
  }  

  general_adc_cfg_diff[num].adc_io = driver_adc_open(ADC_ADS1220,0);
  general_adc_cfg_diff[num].highScale = cfg->highScale;
  general_adc_cfg_diff[num].lowScale  = cfg->lowScale;
  general_adc_cfg_diff[num].scale     = cfg->scale;

  general_adc_diff[num].cfg = &general_adc_cfg_diff[num];

return &general_adc_diff[num];
}


float general_adc_read_single(void *driver,uint8_t *err)
{
  general_adc_cfg_t *cfg = ((driver_t *)driver)->cfg;
  adc_config_t adc_config;

  adc_config.mode      = 0;
  adc_config.channel   = cfg->channel;
  adc_config.highScale = cfg->highScale;
  adc_config.lowScale  = cfg->lowScale;
  adc_config.scale     = cfg->scale;

  return calculate_adc(&adc_config,err);
}

float general_adc_read_diff(void *driver,uint8_t *err)
{
  general_adc_cfg_t *cfg = ((driver_t *)driver)->cfg;
  adc_config_t adc_config;

  adc_config.mode      = 1;
  adc_config.channel   = cfg->channel;
  adc_config.highScale = cfg->highScale;
  adc_config.lowScale  = cfg->lowScale;
  adc_config.scale     = cfg->scale;

  return calculate_adc(&adc_config,err);
}
