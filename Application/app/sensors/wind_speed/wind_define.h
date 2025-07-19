

#ifndef WIND_DEFINE_H

#define WIND_DEFINE_H

#include <stdint.h>

typedef enum
{
    eWIND_SET,
} wind_set_option_t;


typedef struct
{
  float (*read)(void *driver,uint8_t type,uint8_t *err);
  void (*set)(void *handle, wind_set_option_t option, void *value);
}wind_api_t;


#endif