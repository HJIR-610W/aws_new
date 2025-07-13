




#include "drv_di.h"
#include "rain_present.h"

driver_t *rainPresent_open(int32_t num,void *opt)
{
  driver_t *driver=NULL;


  return driver;

}


bool read_sensor_rainPresent(driver_t *driver,uint8_t *err)
{
  bool data=true;

  if (drv_di_read(DI_RAIN_DETECT>0))
  {
    data = false;
  }

  return data;
}