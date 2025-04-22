
#ifndef TASK_MEASURE_H
#define TASK_MEASURE_H


#include "app_sensor.h"

typedef enum data_type_e
{
  eDATA_TYPE_I,
  eDATA_TYPE_F,
  eDATA_TYPE_B
} eDATA_TYPE_t;

typedef struct sensor_data_s
{
  union aws_data
  {
    int32_t i;
    float f;
    bool b;
  } data;
  union
  {
    int32_t i;
    float f;
  } min;
  union
  {
    int32_t i;
    float f;
  } max;
  eDATA_TYPE_t data_type;
  uint8_t err;
  uint8_t enable : 1;
} sensor_data_t;

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