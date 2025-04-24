

#ifndef PT100_H
#define PT100_H

#include <stdint.h>
#include "temperature_define.h"


#define PT100_A 0 //ΩÃ±€√§≥Œ 16 ∞Ì¡§µ  
#define PT100_B 1 //ΩÃ±€√§≥Œ 17 ∞Ì¡§µ 

void *pt100_open(uint8_t num,void *opt);

#endif