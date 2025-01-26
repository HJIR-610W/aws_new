
#ifndef HJ_SNOW_H
#define HJ_SNOW_H

#include "dev_io.h"

  int32_t read_hjSnowFall(dev_io_t *dev,uint8_t *err);
    void hjsnow_init(dev_io_t *io);
#endif