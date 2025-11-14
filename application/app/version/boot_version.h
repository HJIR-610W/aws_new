
#ifndef BOOT_VERSION_H
#define BOOT_VERSION_H

#include <stdint.h>

#include "util_time.h"

uint32_t get_boot_version(uint8_t *a,uint8_t *b,uint8_t *c,uint8_t *d);
void get_boot_build(DATE_TIME_BUF *build);
uint32_t get_boot_build_timestamp(void);
    uint32_t get_boot_pcb_version(void);
#endif