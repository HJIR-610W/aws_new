

#include "config_adc.h"
#include "config_app.h"
#include "config_nvm.h"
#include "config_sensor.h"

#include "drv_fram.h"

void config_manager_init(void)
{
  load_config_app();

  load_config_nvm();
  load_config_sensor();
  load_adc_cali();
}


void backup_config(void)
{
  backup_config_app();
  backup_config_sensor();
}

void restore_config(void)
{
  restore_config_app();
  restore_config_sensor();
}