
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "app_sensor.h"
#include "driver_interface.h"

#include "general_adc.h"
#include "drv_adc.h"
#include "app_adc.h"



typedef struct general_adc_cfg_s
{
  int adc_num;
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
    if(general_adc_single[cfg->single_channel].opened)
    {
      return &general_adc_single[cfg->single_channel];
    }

    general_adc_cfg_single[cfg->single_channel].adc_num = cfg->single_channel;
    general_adc_cfg_single[cfg->single_channel].highScale = cfg->highScale;
    general_adc_cfg_single[cfg->single_channel].lowScale  = cfg->lowScale;
    general_adc_cfg_single[cfg->single_channel].scale     = cfg->scale;
    general_adc_cfg_single[cfg->single_channel].channel   = cfg->single_channel;
    general_adc_cfg_single[cfg->single_channel].mode      = cfg->mode;
    general_adc_cfg_single[cfg->single_channel].outMaxVolt      = cfg->outMaxV;
    general_adc_cfg_single[cfg->single_channel].outMinVolt      = cfg->outMinV;
    general_adc_single[cfg->single_channel].cfg = &general_adc_cfg_single[cfg->single_channel];
    general_adc_single[cfg->single_channel].name = "GENERAL_ADC";
    return &general_adc_single[cfg->single_channel];
  }
  else
  {
    if(general_adc_diff[cfg->diff_channel].opened)
    {
      return &general_adc_diff[cfg->diff_channel];
    }

    general_adc_cfg_diff[cfg->diff_channel].adc_num = cfg->diff_channel;
    general_adc_cfg_diff[cfg->diff_channel].highScale = cfg->highScale;
    general_adc_cfg_diff[cfg->diff_channel].lowScale  = cfg->lowScale;
    general_adc_cfg_diff[cfg->diff_channel].scale     = cfg->scale;
    general_adc_cfg_diff[cfg->diff_channel].channel = cfg->diff_channel;
    general_adc_cfg_diff[cfg->diff_channel].mode = cfg->mode;
    general_adc_cfg_diff[cfg->diff_channel].outMaxVolt = cfg->outMaxV;
    general_adc_cfg_diff[cfg->diff_channel].outMinVolt = cfg->outMinV;
    general_adc_diff[cfg->diff_channel].name = "GENERAL_ADC";
    general_adc_diff[cfg->diff_channel].cfg = &general_adc_cfg_diff[cfg->diff_channel];
  
  return &general_adc_diff[cfg->diff_channel];

  }

}



float general_adc_read(void *driver,uint8_t *err)
{
  general_adc_cfg_t *cfg = ((driver_t *)driver)->cfg;
  adc_config_t adc_config;
  
  if(driver == NULL)
  {
    *err = 1;
    return NAN;
  }
  adc_config.mode      = cfg->mode;
  adc_config.single_channel   = cfg->channel;
  adc_config.highScale = cfg->highScale;
  adc_config.lowScale  = cfg->lowScale;
  adc_config.scale     = cfg->scale;
  adc_config.outMaxV   = cfg->outMaxVolt;
  adc_config.outMinV   = cfg->outMinVolt;

  return cvt_voltate_to_data(&adc_config,err);
}

