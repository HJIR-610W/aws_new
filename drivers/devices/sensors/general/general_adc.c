
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
  const char *owner;
}general_adc_cfg_t;


general_adc_cfg_t general_adc_cfg_single[16];
general_adc_cfg_t general_adc_cfg_diff[8];
driver_t general_adc_single[16];
driver_t general_adc_diff[8];




#define ADC_OWNER_SIZE 20
char single_owner_table[16][ADC_OWNER_SIZE]={{"SE 0"},
{"SE 1"},
{"SE 2"},
{"SE 3"},
{"SE 4"},
{"SE 5"},
{"SE 6"},
{"SE 7"},
{"SE 8"},
{"SE 9"},
{"SE10"},
{"SE11"},
{"SE12"},
{"SE13"},
{"SE14"},
{"SE15"}};

char diff_owner_table[8][ADC_OWNER_SIZE]={{"DIFF 0"},
{"DIFF 1"},
{"DIFF 2"},
{"DIFF 3"},
{"DIFF 4"},
{"DIFF 5"},
{"DIFF 6"},
{"DIFF 7"}};


 

const char *g_adc_single_owner_list[16] = {
    single_owner_table[0],
    single_owner_table[1],
    single_owner_table[2],
    single_owner_table[3],
    single_owner_table[4],
    single_owner_table[5],
    single_owner_table[6],
    single_owner_table[7],
    single_owner_table[8],
    single_owner_table[9],
    single_owner_table[10],
    single_owner_table[11],
    single_owner_table[12],
    single_owner_table[13],
    single_owner_table[14],
    single_owner_table[15]
};


const char *g_adc_diff_owner_list[8] = {
    diff_owner_table[0],
    diff_owner_table[1],
    diff_owner_table[2],
    diff_owner_table[3],
    diff_owner_table[4],
    diff_owner_table[5],
    diff_owner_table[6],
    diff_owner_table[7]
};
const char *general_adc_read_owner(int mode,int channel)
{
  if(mode ==0) //single
  {
    snprintf(single_owner_table[channel],ADC_OWNER_SIZE,"SE%2d %s",channel,general_adc_cfg_single[channel].owner);
    
   return single_owner_table[channel];
  }
  else if(mode == 1)
  {
    snprintf(diff_owner_table[channel],ADC_OWNER_SIZE,"DIFF%2d %s",channel,general_adc_cfg_diff[channel].owner);
    
   return diff_owner_table[channel];
  }
    return " ";

}




void *general_adc_open(uint8_t num,void *opt,const char *owner)
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
    general_adc_cfg_single[cfg->single_channel].owner      = owner;
    general_adc_single[cfg->single_channel].cfg = &general_adc_cfg_single[cfg->single_channel];

    general_adc_single[cfg->single_channel].name = "GENERAL_ADC";
    
    general_adc_single[cfg->single_channel].opened = true;
    
    
    general_adc_read_owner(cfg->mode,cfg->single_channel);//채널 소유자 테이블 업데이트
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
    general_adc_cfg_diff[cfg->diff_channel].owner      = owner;
    general_adc_diff[cfg->diff_channel].name = "GENERAL_ADC";
    general_adc_diff[cfg->diff_channel].cfg = &general_adc_cfg_diff[cfg->diff_channel];
  general_adc_diff[cfg->diff_channel].opened = true;
  
      general_adc_read_owner(cfg->mode,cfg->diff_channel);//채널 소유자 테이블 업데이트
      
      
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







