

#ifndef POWER_DEFINE_H
#define POWER_DEFINE_H

#include <stdint.h>
typedef struct power_s
{
  float solarVoltage;
  float solarCurrent;
  float batteryVoltage1;
  float batteryVoltage2;
  float loadVoltage1;
  float load1Current;
  float load2Current;
  float load3Current;

  float loadCurrentN;
}power_t;

#endif

