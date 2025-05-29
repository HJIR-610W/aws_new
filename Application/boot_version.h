
#ifndef BOOT_VERSION_H
#define BOOT_VERSION_H

#include <stdint.h>

#include "utile_time.h"

uint32_t get_bootVer(uint8_t *a,uint8_t *b,uint8_t *c,uint8_t *d);
void get_bootBuild(DATE_TIME_BUF *build);
uint32_t get_bootPCB(void);
#endif