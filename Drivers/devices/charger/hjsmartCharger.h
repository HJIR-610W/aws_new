

#ifndef HJSMART_CHARGER_H
#define HJSMART_CHARGER_H

#include "driver_charger_define.h"


int32_t hj_smartcharger_init(void);
void hjsmartCharger_read(charger_data_t *charger_data, uint8_t *err);
#endif