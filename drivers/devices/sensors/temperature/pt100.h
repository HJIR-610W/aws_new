

#ifndef PT100_H
#define PT100_H

#include <stdint.h>
#include "temperature_define.h"



driver_t *pt100_open(void *opt,const char *owner);

extern const char *g_pt100_owner_list[2];
#endif