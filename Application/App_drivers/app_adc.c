

#include <math.h>

#include "cmsis_os2.h"

#include "app_adc.h"
#include "config.h"
#include "driver_adc.h"
#include "utile.h"

driver_t *g_ads1120;

void adc_init(void)
{
  g_ads1120 = driver_adc_open(ADC_ADS1220,0);
}

int32_t adc_read_single(int channel,uint8_t *err)
{
  return  driver_adc_single_read(g_ads1120,channel,1,err);
}

int32_t adc_read_single_avg(int channel,uint8_t *err,uint8_t avg_cnt)
{

return driver_adc_single_read(g_ads1120,channel,avg_cnt,err);



}


int32_t adc_read_diff_avg(int channel,uint8_t *err,uint8_t avg_cnt)
{

return driver_adc_diff_read(g_ads1120,channel,avg_cnt,err);


}

int32_t adc_read_diff(int channel,uint8_t *err)
{
  return  driver_adc_diff_read(g_ads1120,channel,1,err);
}






float cvt_adcToVol(int32_t adc,int32_t off,int32_t full,int32_t off_in,int32_t full_in)
{
  float gain;
  float voltage;
  gain = (float)(full_in - off_in)/(float)(full-off);

  voltage = gain*(adc-off);

  return voltage;
}


float adc_chToVoltage(int32_t mode,int32_t channel,int32_t adc)
{
  int32_t off,full,o_in,f_in;

  if(mode==0)//single
  {
    off  = g_adc_cali_config.single[channel].offset;
    o_in = g_adc_cali_config.single[channel].offset_input;
    full = g_adc_cali_config.single[channel].fullset;
    f_in = g_adc_cali_config.single[channel].fullset_input;
  }
  else
  {
    off  = g_adc_cali_config.diff[channel].offset;
    o_in = g_adc_cali_config.diff[channel].offset_input;
    full = g_adc_cali_config.diff[channel].fullset;
    f_in = g_adc_cali_config.diff[channel].fullset_input;
  }

  return cvt_adcToVol(adc,off,full,o_in,f_in)/1000.0;
}


float adc_read_volate(adc_config_t *adc,uint8_t *err)
{
  int32_t data;
  float ret;
  if(adc->mode == eSINGLE_ADC)
  {
    data = adc_read_single(adc->channel,err);
    ret= adc_chToVoltage(adc->mode,adc->channel,data);
  }
  else
  {
    data = adc_read_diff(adc->channel,err);
    ret= adc_chToVoltage(adc->mode,adc->channel,data);
  }
  return ret;
}


float adc_read_volate_single(int32_t ch,uint8_t *err)
{
  int32_t data;
  float ret;

    data = adc_read_single(ch,err);
    ret= adc_chToVoltage(eSINGLE_ADC,ch,data);

  return ret;
}




int32_t get_adc_vref(adc_config_t *adc)
{
  if(adc->mode==eSINGLE_ADC)
  {
    return   g_adc_cali_config.single[adc->channel].fullset_input;

  }
  else
  {
    return g_adc_cali_config.diff[adc->channel].fullset_input;
  }
}


float calculate_adc(adc_config_t *adc_config,uint8_t *err)
{
  float val;
  int32_t vref;
  float data;
  float retVal;

  vref = get_adc_vref(adc_config);

  data = adc_read_volate(adc_config,err);


  if(*err)
  {
    return NAN;
  }

  val = adc_config->lowScale + (adc_config->highScale - adc_config->lowScale)*data/(vref/1000.0);
  
  if(val >adc_config->highScale)
  {
    val = adc_config->highScale;
  } 

  if(val < adc_config->lowScale)
  {
    val = adc_config->lowScale;
  }

return val/adc_config->scale;

}


float calculate_voltage(adc_config_t *adc_config,uint8_t *err)
{
  float val;
  int32_t vref;
  float data;
  float retVal;

  vref = adc_config->outMaxV;

  data = adc_read_volate(adc_config,err);


  if(*err)
  {
    return NAN;
  }

  val = adc_config->lowScale + (adc_config->highScale - adc_config->lowScale)*data/(vref/1000.0);
  
  if(val >adc_config->highScale)
  {
    val = adc_config->highScale;
  } 

  if(val < adc_config->lowScale)
  {
    val = adc_config->lowScale;
  }

return val/adc_config->scale;

}

int32_t get_adc_single_offset(int channel)
{
  int32_t offset;

  return g_adc_cali_config.single[channel].offset;
}

int32_t get_adc_single_fullset(int channel)
{
  int32_t offset;

  return g_adc_cali_config.single[channel].fullset;
}