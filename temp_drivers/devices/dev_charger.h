#ifndef DEV_CHARGER_H
#define DEV_CHARGER_H

#include <stdint.h>
#include "driver_charger_define.h"
#define DEV_CHARGER_HJ_SMART 0
#define DEV_CHARGER_LS1024 1

int32_t dev_charger_init(int32_t num);
void dev_charger_read(int num, charger_data_t *charger_data, uint8_t *err);
#endif