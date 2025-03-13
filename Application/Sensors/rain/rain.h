
#ifndef RAIN_H
#define RAIN_H

#include <stdbool.h>
#include "app_sensor.h"

typedef struct rain_data_s
{
  uint16_t min;
  uint16_t min10;
  uint16_t hour;
  uint16_t day;
  uint16_t month;
  uint16_t year;
}rain_data_t;

void rain_init(sensor_t *sensor);
int32_t read_sensor_rain(sensor_t *sensor,uint8_t *err);

void rainPresent_init(sensor_t *sensor);
bool is_rainPresentInit(void);
bool rainPresent_deInit(void);

bool read_sensor_rainPresent(sensor_t *sensor,uint8_t *err);

int32_t read_rainHallErr(void);


void increase_rain(void);
uint16_t peek_rain(void);
uint16_t get_rain(uint16_t cnt);


extern rain_data_t rain_data;;
#endif