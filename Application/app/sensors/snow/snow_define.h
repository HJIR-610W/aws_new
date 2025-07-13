

#ifndef SNOW_DEFINE_H

#define SNOW_DEFINE_H


#include <stdint.h>

#include "driver_interface.h"
typedef enum
{
    eTEMP_SET,
} snow_set_option_t;


typedef struct
{
  int32_t (*read)(driver_t *driver,uint8_t *err);
  void (*set)(void *handle, snow_set_option_t option, void *value);
}snow_api_t;

#endif