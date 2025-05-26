

#ifndef SOLAR_RADIATION_DEFINE_H
#define SOLAR_RADIATION_DEFINE_H

#include <stdint.h>

typedef enum
{
    eTEMP_SET,
} solarRadiation_set_option_t;

typedef enum
{
  eSOLAR_RADIATION_INFO_GET,
} solarRadiation_get_option_t;


typedef struct
{
  float (*read)(driver_t *driver,uint8_t *err);
  void (*set)(driver_t *handle, solarRadiation_set_option_t option, void *value);
  void (*get)(driver_t *handle, solarRadiation_get_option_t option, void *value);
} solarRadiation_api_t;

#endif