

#ifndef TEMPERATURE_DEFINE_H

#define TEMPERATURE_DEFINE_H
#include <stdint.h>

#include "driver_interface.h"

typedef struct temperature_set_cfg_s
{
  int32_t channel;
} temperature_set_cfg_t;

typedef enum
{
  eTEMP_SET_OFFSET,
  eHUMI_SET_OFFSET
} temperature_set_option_t;

typedef enum
{
  eTEMP_GET_OFFSET,
  eHUMI_GET_OFFSET
} temperature_get_option_t;

typedef struct
{
  float (*read)(driver_t *driver, uint8_t *err);
  void (*set)(driver_t *handle, temperature_set_option_t option, void *value);
  int32_t (*get)(driver_t *handle, temperature_get_option_t option, void *value);
} temperature_api_t;

#endif