
#ifndef TASK_MEASURE_H
#define TASK_MEASURE_H


#include "app_sensor.h"

typedef enum measure_type_e
{
  eMEASURE_TYPE_250MS,
  eMEASURE_TYPE_1000MS
}eMEASURE_TYPE_t;

 typedef struct measure_data_s
{
  eMEASURE_TYPE_t type;
  sensor_data_t data[SENSOR_LIST_MAX];
} measure_data_t;


void measureTask_init(void);

bool is_measurement(void *data);

#endif