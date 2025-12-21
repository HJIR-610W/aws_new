#ifndef TASK_SYSTEM_H
#define TASK_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>



typedef enum config_app_field_e
{
  eCONFIG_APP_SENSOR
} eCONFIG_APP_FIELD_t;

typedef enum AC_status_e
{
  eAC_100V,
  eAC_220V,
  eAC_OFF
}eAC_STATUS_t;

typedef struct system_s
{
  bool dc_error;
  bool battery_error;
  bool door_opened;
  bool sdcard_inserted;
  eAC_STATUS_t ac_status; 
  bool fan_active;
  float charger_solar1_voltage;
  float charger_solar2_voltage;
  float charger_solar1_currnet;
  float charger_solar2_currnet;
  float charger_battery1_voltage;
  float charger_battery2_voltage;
  float charger_load1_currnet;
  float charger_load2_currnet;
  float charger_load3_currnet;
  float battery_voltage;
}system_t;


#define PARA_RUN_MODE 0
#define PARA_TEST_MODE 1
void systemTask_init(uint32_t para);
int is_door_opened(void);
system_t *get_system(void);
extern system_t System;
#endif