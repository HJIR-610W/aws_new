
#ifndef DRIVER_CHARGER_DEFINE_H
#define DRIVER_CHARGER_DEFINE_H

#include "driver_interface.h"

#define CHARGER_ERR_RECV_TIMEOUT 2
#define CHARGER_ERR_RECV_PACKET  3

typedef struct charger_data_s
{
  float solar1Volt;
  float solar1Current;
  float solar2Volt;
  float solar2Current;
  float battery1;
  float battery2;
  float load1Current;
  float load2Current;
  float load3Current;
}charger_data_t;

typedef struct
{
    void (*read)(driver_t *driver,charger_data_t *data,uint8_t *err);
}charger_api_t;

#endif