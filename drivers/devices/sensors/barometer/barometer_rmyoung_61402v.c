

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
typedef struct rmyoung_61402v_instance_s
{
  adc_config_t adc_config;
  bool opened;
} rmyoung_61402v_instance_t;

rmyoung_61402v_instance_t rmyoung_61402v_inst;


int32_t rmyoung_61402v_init(void *opt)
{
  rmyoung_61402v_barometer_config_t  *p_cfg = (rmyoung_61402v_barometer_config_t *)opt;
  uart_config_t uart_config;

  if (rmyoung_61402v_inst.opened)
  {
    return 1;
  }

  rmyoung_61402v_inst.opened = true;
  rmyoung_61402v_inst.adc_config.mode = 0;
  rmyoung_61402v_inst.adc_config.single_channel = p_cfg->adc_channel;
  rmyoung_61402v_inst.adc_config.highScale = 1100;
  rmyoung_61402v_inst.adc_config.lowScale = 500;
  rmyoung_61402v_inst.adc_config.outMaxV = 5000;
  rmyoung_61402v_inst.adc_config.outMinV = 0;
  rmyoung_61402v_inst.adc_config.scale = 1;

  return 1;
}

float read_baromater_rmyoung_61402v(uint8_t *err)
{
  float barometer = NAN;

  barometer = cvt_voltate_to_data(&rmyoung_61402v_inst.adc_config, err);

  return barometer;
}
