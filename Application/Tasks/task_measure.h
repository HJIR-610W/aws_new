
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
  } offset;
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

//250ms¸¶´Ù ¼öÁýÇÏ´Â µ¥ÀÌÅÍ
typedef enum reading_250
{
  eA2_WIND_DIRECTION,
  eA3_WIND_SPEED
}eREADING_250MS_t;

typedef struct measure_data_250ms
{
  sensor_data_t data[2];//Ç³Çâ Ç³¼Ó
} measure_data_250ms_t;

typedef struct measure_data_1s
{
  sensor_data_t data[SENSOR_LIST_MAX];//Ç³Çâ Ç³¼Ó ÀÎµ¦½º´Â ¹Ì»ç¿ë
} measure_data_1s_t;



typedef struct
{
  uint32_t start_time;
  uint32_t elapsed_time;
  uint32_t elapsed_max;
} exec_time_t;

void measureTask_init(void);

bool is_measurement_1s( void *data,uint32_t timeout);
bool is_measurement_250(void *data, uint32_t timeout);

extern exec_time_t g_exec_250ms_time;
extern exec_time_t g_exec_1s_time;



#endif