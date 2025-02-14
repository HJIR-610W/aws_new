
#include "power.h"
#include "smart_charger.h"
#include "ls1024.h"


uint8_t powerModel;

void power_init(uint8_t model)
{
  
}

int32_t read_power(uint8_t model,power_t *power)
{
  int32_t status=0;

  switch (model)
  {
  case POWER_LS1024:
    read_ls1024(power);
    break;
  case POWER_SMART_CHARGER:
  default:
    read_smartCharger(power);
    break;
  }

  return status;
}