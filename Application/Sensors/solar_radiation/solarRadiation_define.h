

#ifndef SOLAR_RADIATION_DEFINE_H
#define SOLAR_RADIATION_DEFINE_H

#include <stdint.h>

typedef enum
{
    eTEMP_SET,
} solarRadiation_set_option_t;


typedef struct
{
  float (*read)(void *driver,uint8_t *err);
  void (*set)(void *handle, solarRadiation_set_option_t option, void *value);
}solarRadiation_api_t;

#endif