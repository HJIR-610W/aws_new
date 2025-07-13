

#ifndef PT100_H
#define PT100_H

#include <stdint.h>
#include "temperature_define.h"


#define PT100_A 0 //?깃?梨꾨꼸 16 怨좎젙??
#define PT100_B 1 //?깃?梨꾨꼸 17 怨좎젙??

void *pt100_open(uint8_t num,void *opt);

#endif