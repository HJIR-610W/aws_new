

#include <math.h>

#include "cmsis_os2.h"

#include "app_adc.h"
#include "config_adc.h"
#include "driver_adc.h"
#include "util_memory.h"

driver_t *g_ads1120;

void adc_init(void)
{
  g_ads1120 = driver_adc_open(ADC_ADS1220,0);
}
#define ADC_AVG_CNT 1

float adc_read_single(int channel,uint8_t *err)
{
  return  driver_adc_single_read(g_ads1120,channel,1,err);
}

float adc_read_single_avg(int channel, uint8_t *err, uint8_t avg_cnt)
{
  return driver_adc_single_read(g_ads1120, channel, avg_cnt, err);
}


#define ADC_RAW_AVG_CNT 1
float adc_read_single_raw(int channel, uint8_t *err)
{
  return driver_adc_single_raw_read(g_ads1120, channel, ADC_RAW_AVG_CNT, err);
}




float adc_read_diff_avg(int channel,uint8_t *err,uint8_t avg_cnt)
{

return driver_adc_diff_read(g_ads1120,channel,avg_cnt,err);


}


int32_t adc_read_diff_raw(int channel, uint8_t *err)
{
  return driver_adc_diff_raw_read(g_ads1120, channel, ADC_RAW_AVG_CNT, err);
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
    off  = g_config_adc.single[channel].offset;
    o_in = g_config_adc.single[channel].offset_input;
    full = g_config_adc.single[channel].fullset;
    f_in = g_config_adc.single[channel].fullset_input;
  }
  else
  {
    off  = g_config_adc.diff[channel].offset;
    o_in = g_config_adc.diff[channel].offset_input;
    full = g_config_adc.diff[channel].fullset;
    f_in = g_config_adc.diff[channel].fullset_input;
  }

  return cvt_adcToVol(adc,off,full,o_in,f_in)/1000.0;
}

#define GENERAL_ADC_AVG_CNT 1
float adc_read_volate(adc_config_t *adc,uint8_t *err)
{
  float voltage=0;
  if(adc->mode == eSINGLE_ADC)
  {
    voltage = adc_read_single_avg(adc->channel, err, GENERAL_ADC_AVG_CNT);
  }
  else
  {
    voltage = adc_read_diff_avg(adc->channel, err, GENERAL_ADC_AVG_CNT);
  }
  return voltage;
}


float adc_read_volate_single(int32_t ch,uint8_t *err)
{
  return adc_read_single(ch,err);
}

float adc_read_volate_single_avg(int32_t ch,uint8_t *err)
{
  return adc_read_single(ch,err);
}





int32_t get_adc_vref(adc_config_t *adc)
{
  if(adc->mode==eSINGLE_ADC)
  {
    return   g_config_adc.single[adc->channel].fullset_input;

  }
  else
  {
    return g_config_adc.diff[adc->channel].fullset_input;
  }
}






float cvt_voltate_to_data(adc_config_t *adc_config,uint8_t *err)
{
  float slope;
  float offset;
  float data;
  float sensor_value;
  float scale;
  float input;

  //y = slope*측정값+오프셋

  scale = (float)(adc_config->highScale - adc_config->lowScale)/(float)adc_config->scale;
  input = (float)(adc_config->outMaxV - adc_config->outMinV)/1000.0f;

  slope = scale/input;

  offset = ((float)adc_config->lowScale/(float)adc_config->scale) -slope*((float)adc_config->outMinV/1000.0); 

  data = adc_read_volate(adc_config,err);

  sensor_value = slope*data+offset;

  if(*err)
  {
    return NAN;
  }

  return sensor_value;

}

float cvt_data_to_voltage(adc_config_t *adc_config, float sensor_value)
{
  float slope;
  float offset;
  float scale;
  float input;
  float voltage;

  // scale = (high - low) / scale값
  scale = (float)(adc_config->highScale - adc_config->lowScale) / (float)adc_config->scale;
  input = (float)(adc_config->outMaxV - adc_config->outMinV) / 1000.0f;

  // 기존 전압 센서값 변환 수식 기준의 역산
  slope = scale / input;
  offset = ((float)adc_config->lowScale / (float)adc_config->scale) -
           slope * ((float)adc_config->outMinV / 1000.0f);

  // 역함수로 전압 계산
  voltage = (sensor_value - offset) / slope;

  return voltage;  // 단위: V
}




void adc_set_offset_trim(int mode,int ch,float offset)
{
  adc_offset_trim_t adc_offset_trim;

  adc_offset_trim.mode = mode;
  adc_offset_trim.channel = ch;
  adc_offset_trim.offset = offset;

  driver_adc_set(g_ads1120, eADC_SET_OFFSET, &adc_offset_trim);
}

bool adc_get_offset_trim(int mode,int ch,float *offset)
{
  adc_offset_trim_t adc_offset_trim;

  adc_offset_trim.mode = mode;
  adc_offset_trim.channel = ch;

  return driver_adc_get(g_ads1120, eADC_GET_OFFSET,&adc_offset_trim,&offset);
}