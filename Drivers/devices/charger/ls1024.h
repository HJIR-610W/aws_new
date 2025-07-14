

#ifndef CHARGER_LS1024_H
#define CHARGER_LS1024_H

#include "driver_charger_define.h"

#define LS1024_CHARGER 0 
driver_t *ls1024_open(int32_t num,void *opt);

#endif