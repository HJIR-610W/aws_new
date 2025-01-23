



#include "driver_adc.h"
#include "cmsis_os.h"
#include "app_adc.h"

#include "config.h"
driver_t *g_adc_s;
driver_t *g_adc_d;
const uint8_t user_adc_single_channel[18]={0,1,4,5,6,9,12,13,16,17,20,21,24,25,28,29,2,6};

void adc_init(void)
{
    g_adc_s = driver_adc_open(ADC_ADS1220_SINGLE_CH_0);
    g_adc_d = driver_adc_open(ADC_ADS1220_DIFF_CH_0);
}

int32_t adc_read_single(int channel,uint8_t *err)
{
  
  return  driver_adc_read(g_adc_s,user_adc_single_channel[channel],err);
}
int32_t adc_read_single_avg(int channel,uint8_t *err,uint8_t avg_cnt)
{
  return  driver_adc_read_average(g_adc_s,user_adc_single_channel[channel],err,avg_cnt);
}


int32_t adc_read_diff_avg(int channel,uint8_t *err,uint8_t avg_cnt)
{
  return  driver_adc_read_average(g_adc_d,channel+ADC_ADS1220_DIFF_CH_0,err,avg_cnt);
}

int32_t adc_read_diff(int channel,uint8_t *err)
{
  return  driver_adc_read(g_adc_d,channel+ADC_ADS1220_DIFF_CH_0,err);
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
    data = adc_read_single(adc->channel-1,err);
    ret= adc_chToVoltage(adc->mode,adc->channel-1,data);
  }
  else
  {
    data = adc_read_diff(adc->channel-1,err);
    ret= adc_chToVoltage(adc->mode,adc->channel-1,data);
  }
  return ret;
}


int32_t get_adc_vref(adc_config_t *adc)
{
  if(adc->mode==eSINGLE_ADC)
  {
    return   g_adc_cali_config.single[adc->channel-1].fullset_input;

  }
  else
  {
    return g_adc_cali_config.diff[adc->channel-1].fullset_input;
  }
}


float calculate_adc(adc_config_t *adc_config,uint8_t *err)
{
  float val;
  int32_t vref;
 float data;

  vref = get_adc_vref(adc_config);
  data = adc_read_volate(adc_config,err);
  val = adc_config->lowScale + (adc_config->highScale - adc_config->lowScale)*data/(vref/1000.0);

return val;

}