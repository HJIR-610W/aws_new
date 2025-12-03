

#include "app_adc.h"

#include <math.h>

#include "cmsis_os2.h"
#include "config_adc.h"
#include "drv_adc.h"
#include "util_memory.h"



#define GENERAL_ADC_AVG_CNT 1


float adc_read_volate(adc_config_t *adc,uint8_t *err)
{
  float voltage=0;
  if(adc->mode == eSINGLE_ADC)
  {
    voltage = drv_adc_single_read_voltage(adc->single_channel,  GENERAL_ADC_AVG_CNT,err);
  }
  else
  {
    voltage = drv_adc_diff_read_voltage(adc->diff_channel,  GENERAL_ADC_AVG_CNT,err);
  }
  return voltage;
}



float cvt_voltate_to_data(adc_config_t *adc_config,uint8_t *err)
{
  float slope;
  float offset;
  float data;
  float sensor_value;
  float scale;
  float input;

  //y = slope*������+������

  scale = (float)(adc_config->high_scale - adc_config->low_scale)/(float)adc_config->scale;
  input = (float)(adc_config->out_max_mv - adc_config->out_min_mv)/1000.0f;

  slope = scale/input;

  offset = ((float)adc_config->low_scale/(float)adc_config->scale) -slope*((float)adc_config->out_min_mv/1000.0); 

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

  // scale = (high - low) / scale��
  scale = (float)(adc_config->high_scale - adc_config->low_scale) / (float)adc_config->scale;
  input = (float)(adc_config->out_max_mv - adc_config->out_min_mv) / 1000.0f;

  // ���� ���� ������ ��ȯ ���� ������ ����
  slope = scale / input;
  offset = ((float)adc_config->low_scale / (float)adc_config->scale) -
           slope * ((float)adc_config->out_min_mv / 1000.0f);

  // ���Լ��� ���� ���
  voltage = (sensor_value - offset) / slope;

  return voltage;  // ����: V
}



