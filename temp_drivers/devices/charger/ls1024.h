

#ifndef CHARGER_LS1024_H
#define CHARGER_LS1024_H

#include "driver_charger_define.h"


void ls1024_read(charger_data_t *charger_data, uint8_t *err);
int32_t ls1024_init(void);
#endif