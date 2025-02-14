
#ifndef POWER_H_
#define POWER_H_

#include <stdint.h>

#include "power_define.h"

#define POWER_LS1024 0
#define POWER_SMART_CHARGER 1
void power_init(uint8_t model);
int32_t read_power(uint8_t model,power_t *power);

#endif