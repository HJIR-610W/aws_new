

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

#include "driver_interface.h"
#include "Sensors\general\general_adc.h"


typedef struct rmyoung_61402v_instance_s
{
  driver_t *driver;
  bool opened;
} rmyoung_61402v_instance_t;

rmyoung_61402v_instance_t rmyoung_61402v_inst;


int32_t rmyoung_61402v_init(void *opt)
{

  adc_config_t adc_config;
      barometer_rmyoung_61402v_config_t  *p_cfg = (barometer_rmyoung_61402v_config_t *)opt;
    if (rmyoung_61402v_inst.opened)
  {
    return 1;
  }

  rmyoung_61402v_inst.opened = true;
  
  
  adc_config.mode = 0;
  adc_config.single_channel = p_cfg->adc_channel;
  adc_config.high_scale = 1100;
  adc_config.low_scale = 500;
  adc_config.out_max_mv = 5000;
  adc_config.out_min_mv = 0;
  adc_config.scale = 1;
  
  
  rmyoung_61402v_inst.driver = general_adc_open(GENERAL_ADC,&adc_config,"Pressure");
  
  return   1;
}

float read_baromater_rmyoung_61402v(uint8_t *err)
{
  float barometer = NAN;


  barometer = general_adc_read(rmyoung_61402v_inst.driver,err);
  
  return barometer;
}
