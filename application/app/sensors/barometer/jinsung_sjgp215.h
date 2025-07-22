
#ifndef JINSUNG_SJGP215_H
#define JINSUNG_SJGP215_H

#include "driver_interface.h"

#include <stdint.h>
int32_t sjgp215_init(void *opt);
float read_sjgp215_baromater(uint8_t *err);
#endif