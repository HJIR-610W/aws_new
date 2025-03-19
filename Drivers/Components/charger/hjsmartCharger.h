

#ifndef HJSMART_CHARGER_H
#define HJSMART_CHARGER_H

#include "driver_charger_define.h"

#define HJ_SMART_CHARGER 0 
driver_t *hjsmartCharger_open(int32_t num,void *opt);

#endif