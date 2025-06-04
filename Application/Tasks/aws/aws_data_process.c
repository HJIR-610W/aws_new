

#include <stdint.h>

#include "app_file.h"
#include "user_heap.h"
#include "util_time.h"


typedef struct monthdata_s
{
  uint16_t data[31];
}monthdata_t;

monthdata_t month[12];

