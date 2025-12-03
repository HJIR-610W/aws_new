

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "drv_rs232.h"

#include "app_sensor.h"
#include "config_sensor.h"
#include "os_user_def.h"
#include "drv_adc.h"
#include "app_adc.h"
#include "Sensors\general\general_adc.h"
#include "driver_interface.h"



typedef struct solar_duration_csd3_instance_s
{
  driver_t *driver;
  bool opened;
} solar_duration_csd3_instance_t;

solar_duration_csd3_instance_t csd3_inst;

int32_t solar_duration_csd3_init(void *opt)
{
  
  adc_config_t adc_config;
  solar_duration_csd3_t *p_cfg = (solar_duration_csd3_t *)opt;
      
  csd3_inst.opened = true;
  adc_config.mode = 0;
  adc_config.single_channel = p_cfg->adc_channel;
  adc_config.high_scale = 5;
  adc_config.low_scale = 0;
  adc_config.out_max_mv = 5000;
  adc_config.out_min_mv = 0;
  adc_config.scale = 1;
  
  csd3_inst.driver = general_adc_open(GENERAL_ADC,&adc_config,"Sunshine");
    
  return 1;
}

float read_solar_duration_csd3(uint8_t *err)
{
  float solar_duration = NAN;

  solar_duration = general_adc_read(csd3_inst.driver, err);

  return solar_duration;
}
