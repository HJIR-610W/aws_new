

#ifndef TEMPERATURE_DEFINE_H

#define TEMPERATURE_DEFINE_H
#include <stdint.h>

#include "driver_interface.h"
#include "hj_temperature_define.h"
typedef struct temperature_set_cfg_s
{
  int32_t channel;
} temperature_set_cfg_t;

typedef struct temperature_read_config_s
{
  hjtemp_register_map_t map;
} temperature_read_config_t;


    typedef enum hjsnow_ctrl_e {
      eTEMP_READ_CONFIG,
      eTEMP_SET_OFFSET,
      eHUMI_SET_OFFSET,
      eTEMP_GET_OFFSET,
      eHUMI_GET_OFFSET
    } eHJTEMPERATURE_OPT_t;

typedef struct
{
  float (*read)(driver_t *driver, uint8_t *err);
  void (*ctrl)(driver_t *driver, eHJTEMPERATURE_OPT_t ctrl, void *w_opt, void *r_opt, uint8_t *err);
} temperature_api_t;

#endif