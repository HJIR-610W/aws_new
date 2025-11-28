

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


typedef struct rmyoung_05103v_win_dir_instance_s
{
  driver_t *driver;
  bool opened;
} rmyoung_05103v_win_dir_instance_t;

rmyoung_05103v_win_dir_instance_t rmyoung_05103v_win_dir_inst;

int32_t wind_dir_rmyoung_05103v_init(void *opt)
{
  rmyoung_05103v_wind_direction_config_t *p_cfg = (rmyoung_05103v_wind_direction_config_t *)opt;
 adc_config_t adc_config;

  if (rmyoung_05103v_win_dir_inst.opened)
  {
    return 1;
  }

  rmyoung_05103v_win_dir_inst.opened = true;
  adc_config.mode = 0;
  adc_config.single_channel = p_cfg->adc_channel;
  adc_config.highScale = 355;
  adc_config.lowScale = 0;
  adc_config.outMaxV = 5000;
  adc_config.outMinV = 0;
  adc_config.scale = 1;

  rmyoung_05103v_win_dir_inst.driver = general_adc_open(GENERAL_ADC,&adc_config,"Wind Direction");
    
  return 1;
}

float read_wind_dir_rmyoung_05103v(uint8_t *err)
{
  float wind_dir = NAN;

  wind_dir = general_adc_read(rmyoung_05103v_win_dir_inst.driver,err);

  return wind_dir;
}
