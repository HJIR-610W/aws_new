
#ifndef DRIVER_CHARGER_H
#define DRIVER_CHARGER_H

#include "driver_interface.h"
#include "driver_charger_define.h"

#define CHARGER_HJ_SMART 0
#define CHARGER_LS       1

driver_t *driver_charger_open(int32_t num,void *opt);
int32_t driver_charger_read(driver_t *driver,charger_data_t *data,uint8_t *err);

#endif