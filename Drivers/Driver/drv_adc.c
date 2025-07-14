
#include "drv_adc.h"


#include "bsp_adc.h"




void drv_adc_init(void)
{
  bsp_adc_init();
}

float drv_adc_single_read_voltage(int channel, uint16_t avg, uint8_t *err)
{
  return bsp_adc_single_read_voltage(channel,avg,err);

}

float drv_adc_diff_read_voltage(int channel, uint16_t avg, uint8_t *err)
{
  return bsp_adc_diff_read_voltage(channel, avg, err);
}


int32_t drv_adc_single_raw_read( int channel, uint16_t avg, uint8_t *err)
{


  return bsp_adc_single_raw_read(channel, avg, err);
}
int32_t drv_adc_diff_raw_read( int channel, uint16_t avg, uint8_t *err)
{

  return bsp_adc_diff_raw_read( channel,  avg, err);
}

void drv_adc_set_offset(int channel,  float offset)
{

  bsp_adc_set_offset(channel,offset);
   

}

bool driver_adc_get(int channel,float *p_offset)
{
   return bsp_adc_get_offset(channel,p_offset);
}