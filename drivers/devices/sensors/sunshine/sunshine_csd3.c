

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
typedef struct solar_duration_csd3_instance_s
{
  adc_config_t adc_config;
  bool opened;
} solar_duration_csd3_instance_t;

solar_duration_csd3_instance_t csd3_inst;

int32_t solar_duration_csd3_init(void *opt)
{
  solar_duration_csd3_t *p_cfg = (solar_duration_csd3_t *)opt;


  if (csd3_inst.opened)
  {
    return 1;
  }

  csd3_inst.opened = true;
  csd3_inst.adc_config.mode = 0;
  csd3_inst.adc_config.single_channel = p_cfg->adc_channel;
  csd3_inst.adc_config.highScale = 5;
  csd3_inst.adc_config.lowScale = 0;
  csd3_inst.adc_config.outMaxV = 5000;
  csd3_inst.adc_config.outMinV = 0;
  csd3_inst.adc_config.scale = 1;

  return 1;
}

float read_solar_duration_csd3(uint8_t *err)
{
  float solar_duration = NAN;

  solar_duration = cvt_voltate_to_data(&csd3_inst.adc_config, err);

  return solar_duration;
}
