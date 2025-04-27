

#ifndef DUAL_PORT_H
#define DUAL_PORT_H

#include "aws_data.h"

typedef enum aws_data_min_s
{
  eAWS_DATA_REAL,
  eAWS_DATA_1MIN,
  eAWS_DATA_10MIN,
  eAWS_DATA_HOUR
} eAWS_DATA_MIN_t;

void dualportTask_init(void);
kma_data_ex_t *get_kma_data(eAWS_DATA_MIN_t min);
#endif