

#ifndef PT100_H
#define PT100_H

#include <stdint.h>
#include "temperature_define.h"


#define PT100_A 0 //싱글채널 16 고정됨 
#define PT100_B 1 //싱글채널 17 고정됨

void *pt100_open(uint8_t num,void *opt);

#endif