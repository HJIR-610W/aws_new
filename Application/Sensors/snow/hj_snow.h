
#ifndef HJ_SNOW_H
#define HJ_SNOW_H

#include "dev_io.h"

#define HJ_SNOW_485 0
#define HJ_SNOW_232 1

driver_t *hjsnow_open(int32_t num,void *opt);

  
#endif