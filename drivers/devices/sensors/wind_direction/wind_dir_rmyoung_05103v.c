

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
typedef struct rmyoung_05103v_win_dir_instance_s
{
  adc_config_t adc_config;
  bool opened;
} rmyoung_05103v_win_dir_instance_t;

rmyoung_05103v_win_dir_instance_t rmyoung_05103v_win_dir_inst;

int32_t wind_dir_rmyoung_05103v_init(void *opt)
{
  rmyoung_05103v_wind_direction_config_t *p_cfg = (rmyoung_05103v_wind_direction_config_t *)opt;
  uart_config_t uart_config;

  if (rmyoung_05103v_win_dir_inst.opened)
  {
    return 1;
  }

  rmyoung_05103v_win_dir_inst.opened = true;
  rmyoung_05103v_win_dir_inst.adc_config.mode = 0;
  rmyoung_05103v_win_dir_inst.adc_config.single_channel = p_cfg->adc_channel;
  rmyoung_05103v_win_dir_inst.adc_config.highScale = 355;
  rmyoung_05103v_win_dir_inst.adc_config.lowScale = 0;
  rmyoung_05103v_win_dir_inst.adc_config.outMaxV = 5000;
  rmyoung_05103v_win_dir_inst.adc_config.outMinV = 0;
  rmyoung_05103v_win_dir_inst.adc_config.scale = 1;

  return 1;
}

float read_wind_dir_rmyoung_05103v(uint8_t *err)
{
  float barometer = NAN;

  barometer = cvt_voltate_to_data(&rmyoung_05103v_win_dir_inst.adc_config, err);

  return barometer;
}
