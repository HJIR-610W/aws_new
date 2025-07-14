



#include "bsp_adc.h"

#include <stdint.h>

#include "adc_calibration.h"
#include "ads1220.h"
#include "driver_adc_define.h"
#include "driver_stm32_adc.h"
void bsp_adc_init(void)
{
  stm32_adc_init();
  ads1220_init();
}

int32_t cvt_ch(int ch)
{
  if (ch == BSP_ADC_STM32_SE_CH_0)
    return STM32_ADC_SE_CH_0;
  if (ch == BSP_ADC_STM32_SE_CH_1)
    return STM32_ADC_SE_CH_1;
  
  return STM32_ADC_SE_CH_0;
}
float bsp_adc_single_read_voltage(int channel, uint16_t avg, uint8_t *err)
{
  int32_t raw_now;
  float voltage;
  switch (channel)
  {
    case BSP_ADC_STM32_SE_CH_0:
    case BSP_ADC_STM32_SE_CH_1:
      raw_now = stm32_adc_read_single(cvt_ch(channel), avg, err);
      voltage = adc_driver_get_value(get_adc_config(1), ADC_CHANNEL_TYPE_SINGLE_ENDED,
                                     cvt_ch(channel), raw_now);
      break;
    case BSP_ADS1220_S_CH_0:
    case BSP_ADS1220_S_CH_1:
    case BSP_ADS1220_S_CH_2:
    case BSP_ADS1220_S_CH_3:
    case BSP_ADS1220_S_CH_4:
    case BSP_ADS1220_S_CH_5:
    case BSP_ADS1220_S_CH_6:
    case BSP_ADS1220_S_CH_7:
    case BSP_ADS1220_S_CH_8:
    case BSP_ADS1220_S_CH_9:
    case BSP_ADS1220_S_CH_10:
    case BSP_ADS1220_S_CH_11:
    case BSP_ADS1220_S_CH_12:
    case BSP_ADS1220_S_CH_13:
    case BSP_ADS1220_S_CH_14:
    case BSP_ADS1220_S_CH_15:
    case BSP_ADS1220_S_CH_16:
    case BSP_ADS1220_S_CH_17:
      raw_now =  ads1220_single_read(channel,  avg, err);
      voltage =
          adc_driver_get_value(get_adc_config(0), ADC_CHANNEL_TYPE_SINGLE_ENDED, channel, raw_now);
      break;

    default:
      break;
  }





  return voltage;



}

float bsp_adc_diff_read_voltage(int channel, uint16_t avg, uint8_t *err)
{
  int32_t raw_now;
  float voltage;

  switch (channel)
  {
    case BSP_ADS1220_D_CH_0:
    case BSP_ADS1220_D_CH_1:
    case BSP_ADS1220_D_CH_2:
    case BSP_ADS1220_D_CH_3:
    case BSP_ADS1220_D_CH_4:
    case BSP_ADS1220_D_CH_5:
    case BSP_ADS1220_D_CH_6:
    case BSP_ADS1220_D_CH_7:
      raw_now =  ads1220_diff_read(channel, avg, err);
      voltage =
          adc_driver_get_value(get_adc_config(0), ADC_CHANNEL_TYPE_DIFFERENTIAL, channel, raw_now);
      break;

  }
  
  return voltage;
}

int32_t bsp_adc_single_raw_read(int channel, uint16_t avg, uint8_t *err)
{
  int32_t raw_now;

  switch (channel)
  {
    case BSP_ADC_STM32_SE_CH_0:
    case BSP_ADC_STM32_SE_CH_1:
      raw_now = stm32_adc_read_single(cvt_ch(channel), avg, err);

      break;
    case BSP_ADS1220_S_CH_0:
    case BSP_ADS1220_S_CH_1:
    case BSP_ADS1220_S_CH_2:
    case BSP_ADS1220_S_CH_3:
    case BSP_ADS1220_S_CH_4:
    case BSP_ADS1220_S_CH_5:
    case BSP_ADS1220_S_CH_6:
    case BSP_ADS1220_S_CH_7:
    case BSP_ADS1220_S_CH_8:
    case BSP_ADS1220_S_CH_9:
    case BSP_ADS1220_S_CH_10:
    case BSP_ADS1220_S_CH_11:
    case BSP_ADS1220_S_CH_12:
    case BSP_ADS1220_S_CH_13:
    case BSP_ADS1220_S_CH_14:
    case BSP_ADS1220_S_CH_15:
    case BSP_ADS1220_S_CH_16:
    case BSP_ADS1220_S_CH_17:
    raw_now =   ads1220_single_read(channel, avg, err);

      break;

    default:
      break;
  }

      return raw_now;
  }
  
  
  int32_t bsp_adc_diff_raw_read(int channel, uint16_t avg, uint8_t *err)
  {
    int32_t raw_now;


    switch (channel)
    {
      case BSP_ADS1220_D_CH_0:
      case BSP_ADS1220_D_CH_1:
      case BSP_ADS1220_D_CH_2:
      case BSP_ADS1220_D_CH_3:
      case BSP_ADS1220_D_CH_4:
      case BSP_ADS1220_D_CH_5:
      case BSP_ADS1220_D_CH_6:
      case BSP_ADS1220_D_CH_7:
        raw_now = ads1220_diff_read(channel, avg, err);

        break;
    }
    return raw_now;
}

void bsp_adc_set_offset(int channel,  float offset)
{
  switch (channel)
  {
    case BSP_ADS1220_S_CH_0:
    case BSP_ADS1220_S_CH_1:
    case BSP_ADS1220_S_CH_2:
    case BSP_ADS1220_S_CH_3:
    case BSP_ADS1220_S_CH_4:
    case BSP_ADS1220_S_CH_5:
    case BSP_ADS1220_S_CH_6:
    case BSP_ADS1220_S_CH_7:
    case BSP_ADS1220_S_CH_8:
    case BSP_ADS1220_S_CH_9:
    case BSP_ADS1220_S_CH_10:
    case BSP_ADS1220_S_CH_11:
    case BSP_ADS1220_S_CH_12:
    case BSP_ADS1220_S_CH_13:
    case BSP_ADS1220_S_CH_14:
    case BSP_ADS1220_S_CH_15:
    case BSP_ADS1220_S_CH_16:
    case BSP_ADS1220_S_CH_17:
      adc_driver_adjust_offset_trim(get_adc_config(0), ADC_CHANNEL_TYPE_SINGLE_ENDED, channel, offset);
      break;
  }

}

bool bsp_adc_get_offset(int channel,float *p_offset)
{
  float offset;
  bool ret = false;
  switch (channel)
  {
    case BSP_ADS1220_S_CH_0:
    case BSP_ADS1220_S_CH_1:
    case BSP_ADS1220_S_CH_2:
    case BSP_ADS1220_S_CH_3:
    case BSP_ADS1220_S_CH_4:
    case BSP_ADS1220_S_CH_5:
    case BSP_ADS1220_S_CH_6:
    case BSP_ADS1220_S_CH_7:
    case BSP_ADS1220_S_CH_8:
    case BSP_ADS1220_S_CH_9:
    case BSP_ADS1220_S_CH_10:
    case BSP_ADS1220_S_CH_11:
    case BSP_ADS1220_S_CH_12:
    case BSP_ADS1220_S_CH_13:
    case BSP_ADS1220_S_CH_14:
    case BSP_ADS1220_S_CH_15:
    case BSP_ADS1220_S_CH_16:
    case BSP_ADS1220_S_CH_17:
      ret = adc_driver_read_offset_trim(get_adc_config(0), ADC_CHANNEL_TYPE_SINGLE_ENDED, channel, &offset);
      if(ret == 0)
      {
        *p_offset = offset;
      }
      break;
  }

  return ret;
}