
#ifndef HJ_SNOW_H
#define HJ_SNOW_H

#include "dev_io.h"
#include "hjsnow_define.h"
#define HJ_SNOW 0


driver_t *hjsnow_open(void *opt);


typedef struct hjsnow_set_distance_s
{
  uint32_t distance;
} hjsnow_set_distance_t;

typedef struct hjsnow_read_config_s
{
  CONFIG_TypeDef config;
} hjsnow_read_config_t;

typedef struct hjsnow_read_system_s
{
  SYSTEM_TypeDef system;
} hjsnow_read_system_t;


    typedef enum hjsnow_ctrl_s {
      eHJSNOW_SET_DISTANCE,
      eHJSNOW_RUN_ZERO,
      eHJSNOW_GET_CONFIG,
      eHJSNOW_GET_SYSTEM,
      eHJSNOW_RUN_RESET
    } eHJSNOW_CTRL_t;

void hjsnow_ctrl(driver_t *driver, eHJSNOW_CTRL_t ctrl, void *w_opt,void *r_opt,uint8_t *err);
driver_t *hjsnow_opened(void);
#endif