#ifndef __GENERAL_FREQUENCY_H__
#define __GENERAL_FREQUENCY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "driver_interface.h"

#define GENERAL_FREQ 2




driver_t *general_freq_open(int channel,void *opt);
  float general_freq_read(driver_t *drv, uint8_t *err);


#ifdef __cplusplus
}
#endif

#endif // __GENERAL_FREQUENCY_H__
