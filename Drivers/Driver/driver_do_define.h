
#ifndef DRIVER_DO_DEFINE_H
#define DRIVER_DO_DEFINE_H

#include "driver_interface.h"

#define DO_OUT_OD 0
#define DO_OUT_PP 1

#define DO_NO_PULL 0
#define DO_PULL_UP  1
#define DO_PULL_DOWN 2


typedef struct do_set_freq_s
{
  uint8_t low_duty;
  uint32_t freq;
}do_set_freq_t;

typedef enum
{
    DO_SET_SPEED,   
   DO_SET_FREQ
  
} do_set_option_t;

typedef struct do_config_s
{
  uint8_t pullup;
  uint8_t mode;
}do_config_t;

typedef struct
{
    void (*close)(driver_t *handle);
    void (*low)(driver_t *handle);
    void (*high)(driver_t *handle);
    void (*set)(driver_t *handle, do_set_option_t option, void *value);
} do_api_t;
#endif

