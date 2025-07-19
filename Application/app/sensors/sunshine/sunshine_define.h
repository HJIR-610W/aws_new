

#ifndef SUNSHINE_DEFINE_H
#define SUNSHINE_DEFINE_H

#include <stdint.h>

typedef enum
{
    eTEMP_SET,
} sunshine_set_option_t;


typedef struct
{
  float (*read)(void *driver,uint8_t *err);
  void (*set)(void *handle, sunshine_set_option_t option, void *value);
}sunshine_api_t;

#endif