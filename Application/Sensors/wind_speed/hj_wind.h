
#ifndef HJ_WIND_H
#define HJ_WIND_H

#include "dev_io.h"
#include "wind_define.h"

#define HJ_WIND 100  // «≥«‚ «≥º” ∞∞¿Ã √≥∏Æ

#define HJ_WIND_CHANNEL_SPEED 1
#define HJ_WIND_CHANNEL_DIRECTION 2

driver_t *hjwind_open(uint8_t num, void *opt);

#endif