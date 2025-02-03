

#ifndef driver_in_def_h
#define driver_in_def_h

#include "mcu_interrupt.h"
#include "driver_interface.h"


typedef enum trigger_e
{
eDI_FALLING,
eDI_RISING,
eDI_RISING_FALLING
}eDI_TRIGGER_t;


typedef struct di_isr_set_cfg_s
{
  const char *name;
  void (*call)(void *);
  eDI_TRIGGER_t trigger;
  uint16_t prio;
  void *handle;
}di_isr_set_cfg_t;


typedef enum
{
  DI_SET_INTERRUPT,
  DI_SET_PULL_R//풀업, 풀다운   
}di_set_option_t;


typedef struct di_init_s
{
  bool pullup;
}di_init_t;

typedef struct
{
  void (*close)(driver_t *handle);
  int32_t (*read)(driver_t *handle);
  void (*set)(driver_t *handle, di_set_option_t option, void *value);
} di_api_t;


#endif