

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "drv_rs232.h"
#include "driver_interface.h"
#include "app_sensor.h"
#include "config_sensor.h"
#include "os_user_def.h"
#include "drv_adc.h"
#include "app_adc.h"
#include "driver_freqInput.h"
typedef struct rmyoung_05103v_win_spd_instance_s
{
  driver_t *freq_drv;
  uint8_t frequency_channel;
  float factor;
  bool opened;
} rmyoung_05103v_win_spd_instance_t;

rmyoung_05103v_win_spd_instance_t rmyoung_05103v_win_spd_inst;

int32_t wind_spd_rmyoung_05103v_init(void *opt)
{
  rmyoung_05103v_wind_speed_config_t *p_cfg = (rmyoung_05103v_wind_speed_config_t *)opt;


  if (rmyoung_05103v_win_spd_inst.opened)
  {
    return 1;
  }

  rmyoung_05103v_win_spd_inst.opened = true;
  rmyoung_05103v_win_spd_inst.frequency_channel = p_cfg->frequency_channel;
  rmyoung_05103v_win_spd_inst.factor = 0.0978;
  rmyoung_05103v_win_spd_inst.freq_drv = driver_freq_open(p_cfg->frequency_channel);

  return 1;
}

float read_wind_spd_rmyoung_05103v(uint8_t *err)
{
  float wind_speed = NAN;
  float frequency = NAN;

  frequency = driver_freq_read(rmyoung_05103v_win_spd_inst.freq_drv, err);

  if(*err ==0)
  {
    wind_speed = frequency*rmyoung_05103v_win_spd_inst.factor;
  }
  return wind_speed;
}
