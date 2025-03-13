

#ifndef TEMPERATURE_DEFINE_H

#define TEMPERATURE_DEFINE_H


#include <stdint.h>

typedef enum
{
    eWIND_SET,
} temperature_set_option_t;


typedef struct
{
  float (*read)(void *driver,uint8_t *err);
  void (*set)(void *handle, temperature_set_option_t option, void *value);
}temperature_api_t;

#endif