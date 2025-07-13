
#ifndef HJ_HUMINITY_H
#define HJ_HUMINITY_H

#include "driver_interface.h"

#define HJ_HUMINITY 0

driver_t *hjHuminity_open(int32_t num, void *opt);
float hjHuminity_read(driver_t *driver, uint8_t *err);

driver_t *hjHumi_opened(void);
#endif