

#include "dev_charger.h"
#include "hjsmartCharger.h"
#include "ls1024.h"

int32_t dev_charger_init(int32_t num)
{
  int32_t status=-1;

  switch (num)
  {
  case DEV_CHARGER_HJ_SMART:
    status = hj_smartcharger_init();
    break;
  case DEV_CHARGER_LS1024:
    status = ls1024_init();
    break;
  
  default:
    break;
  }

  return status;
}

void dev_charger_read(int num,charger_data_t *charger_data, uint8_t *err)
{

  switch (num)
  {
  case DEV_CHARGER_HJ_SMART:
    hjsmartCharger_read(charger_data,err);
    break;
  case DEV_CHARGER_LS1024:
   ls1024_read(charger_data,err);
    break;

  default:
    break;
  }


}