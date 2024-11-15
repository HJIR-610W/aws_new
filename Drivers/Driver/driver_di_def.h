

#ifndef driver_in_def_h
#define driver_in_def_h

#include "mcu_interrupt.h"

#define DI_SET_INTERRUT 0

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
}di_isr_set_cfg_t;


#endif