
#ifndef RAIN_H
#define RAIN_H

#include <stdbool.h>
#include "app_sensor.h"
#include "driver_interface.h"
typedef struct rain_data_s
{
  uint16_t min;
  uint16_t min10;
  uint16_t hour;
  uint16_t day;
  uint16_t month;
  uint16_t year;
}rain_data_t;


void rainPresent_init(sensor_t *sensor);
bool is_rainPresentInit(void);
bool rainPresent_deInit(void);



int32_t read_rainHallErr(void);


void increase_rain(void);
uint16_t peek_rain(void);
uint16_t get_rain(uint16_t cnt);


extern rain_data_t rain_data;;


#define RAIN_REED_05MM 100
#define RAIN_REED_1MM  101
#define RAIN_HALL_05MM 102
#define RAIN_HALL_1MM  103

driver_t *rain_open(int32_t num,void *opt);
float read_sensor_rain(driver_t *driver,uint8_t *err);

#endif