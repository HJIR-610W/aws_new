#ifndef DATA_LOGGING_H
#define DATA_LOGGING_H

#include <stdint.h>
#include "util_time.h"

typedef enum
{
  eDATA_TYPE_RAIN
} eDATA_TYPE_t;

int32_t save_data(DATE_TIME_BUF* ct, const void* data_ptr, size_t data_size, uint8_t logging_min,
                  eDATA_TYPE_t data_type);

#endif